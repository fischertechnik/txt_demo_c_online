///////////////////////////////////////////////////////////////////////////////
//
// File:    ftProInterface2013SocketCom.cpp
//
// Project: ftPro - fischertechnik Control Graphical Programming System
//
// Module:  TCP/IP communication protocol with interface 2013
//
// Author:  Michael Sögtrop
//
///////////////////////////////////////////////////////////////////////////////
//
// Urheberrecht im Sinne des Urheberrechtsgesetzes bei
//
// Michael Sögtrop
// Germany
//
// Copyright (C) 2014
//
///////////////////////////////////////////////////////////////////////////////
//
// TO DO:
//
///////////////////////////////////////////////////////////////////////////////
//
// Implementation details for module ftProInterface2013SoecketCom
//
// see also:
//
///////////////////////////////////////////////////////////////////////////////

#include <memory.h>
#include <assert.h>

extern "C" {
#include "common.h"

#ifdef WIN32
    typedef unsigned long       UINT32;
#endif

#include "FtShmemTxt.h"
}

#include "ftProInterface2013SocketCom.h"

//******************************************************************************
//****
//**** Class CRC32: Implementation 
//****
//******************************************************************************

CRC32::CRC32() :m_table{}
{
    m_crc=0xffffffff;

    for( UINT32 dividend=0; dividend<256; dividend++ )
    {
        UINT32 remainder = dividend << 24;
        for ( UINT32 bit = 8; bit > 0; bit--)
        {
            if (remainder & 0x80000000)
            {
                remainder = (remainder << 1) ^ 0x04C11DB7;
            }
            else
            {
                remainder = (remainder << 1);
            }
        }
        m_table[dividend] = remainder;
    }
}

void CRC32::Reset()
{
    m_crc=0xffffffff;
}

void CRC32::Add16bit(UINT16 val)
{
    UINT8 data;
    data = (m_crc>>24) ^ (val >> 8);
    m_crc = (m_crc << 8) ^ m_table[data];

    data = (m_crc>>24) ^ (val & 0xff);
    m_crc = (m_crc << 8) ^ m_table[data];
}

//******************************************************************************
//****
//**** Class CompressionBuffer: Implementation 
//****
//******************************************************************************

CompressionBuffer::CompressionBuffer(UINT8 *buffer, int bufsize) :
#ifdef _DEBUG
    m_check_expand(new UINT8[bufsize], bufsize),
#endif
    m_compressed(buffer),
    max_compressed_size(bufsize)
{
    Reset();
}

CompressionBuffer::~CompressionBuffer()
{
#ifdef _DEBUG
    delete [] m_check_expand.GetBuffer();
#endif
}

void CompressionBuffer::Reset()
{
    Rewind();
    memset( m_previous_words, 0, sizeof(m_previous_words) );
    memset( m_compressed, 0, sizeof(m_compressed) );
}

void CompressionBuffer::Rewind()
{
    m_word_count = 0;
    m_compressed_size = 0;
    m_nochange_count = 0;
    m_bitcount = 0;
    m_bitbuffer = 0;
    m_crc.Reset();
#ifdef _DEBUG
    m_check_expand.Rewind();
#endif
}

void CompressionBuffer::PushBits( INT32 count, UINT32 bits )
{
    // byte      |2 2 2 2 2 2 2 2|1 1 1 1 1 1 1 1|
    // fragment  |7 7|6 6|5 5|4 4 4 4|3 3|2 2|1 1|                                 

    m_bitbuffer |= (bits << m_bitcount);
    m_bitcount += count;
    while( m_bitcount >= 8 )
    {
        m_bitcount -= 8;
        m_compressed[m_compressed_size++] = m_bitbuffer & 0xff;
        m_bitbuffer >>= 8;
    }
}

void CompressionBuffer::EncodeNoChangeCount()
{
    // 00 NoChange 1x16 bit
    // 01 00 NoChange 2x16 bit
    // 01 01 NoChange 3x16 bit
    // 01 10 NoChange 4x16 bit
    // 01 11 xxxx NoChange 5..19x16 bit
    // 01 11 1111 xxxxxxxx NoChange 20..274 x16 bit
    // 01 11 1111 11111111 xxxxxxxx-xxxxxxxx NoChange 275... bit

    while(m_nochange_count)
    {
        if(m_nochange_count==1)
        {
            PushBits(2,0);
            break;
        }
        else if(m_nochange_count<=4)
        {
            PushBits(2,1);
            PushBits(2,m_nochange_count-2);
            break;
        }
        else if(m_nochange_count<=4+15)
        {
            PushBits(2,1);
            PushBits(2,3);
            PushBits(4,m_nochange_count-4-1);
            break;
        }
        else if(m_nochange_count<=4+15+255)
        {
            PushBits(2,1);
            PushBits(2,3);
            PushBits(4,15);
            PushBits(8,m_nochange_count-4-15-1);
            break;
        }
        else if(m_nochange_count<=4+15+255+4096)
        {
            PushBits(2,1);
            PushBits(2,3);
            PushBits(4,15);
            PushBits(8,255);
            PushBits(16,m_nochange_count-4-15-255-1);
            break;
        }
        else
        {
            PushBits(2,1);
            PushBits(2,3);
            PushBits(4,15);
            PushBits(8,255);
            PushBits(16,4095);
            m_nochange_count += -4-15-255-4096;
        }
    }
    m_nochange_count = 0;
}

void CompressionBuffer::AddWord(UINT16 word)
{
    m_crc.Add16bit(word);

    assert( m_word_count < max_word_count );
    assert( m_compressed_size < max_compressed_size-8 );

    if( word == m_previous_words[m_word_count] )
    {
        m_nochange_count++;
        m_word_count++;
    }
    else
    {
        EncodeNoChangeCount();
        if(word == 1 && m_previous_words[m_word_count]==0 || word == 0 && m_previous_words[m_word_count]!=0)
        {
            // 10 Toggle (0 to 1, everything else to 0
           PushBits(2,2);
        }
        else
        {
            // 11 16 bit follow immediately
            PushBits(2,3);
            PushBits(16,word);
        }
        m_previous_words[m_word_count]=word;
        m_word_count++;
    }
}

void CompressionBuffer::Finish()
{
    EncodeNoChangeCount();
    if( m_bitcount ) 
    {
        PushBits(8-m_bitcount,0);
    }

#ifdef _DEBUG
    m_check_expand.Rewind();
    memcpy(m_check_expand.GetBuffer(), GetBuffer(), GetCompressedSize());
    m_check_expand.SetBufferSize(GetCompressedSize());
    for( int i=0; i<GetWordCount(); i++ )
    {
        UINT16 v1 = m_check_expand.GetUINT16();
        UINT16 v2 = GetPrevWord(i);
        assert( v1 == v2 );
    }
    assert( m_check_expand.GetCrc() == GetCrc() );
#endif

}

//******************************************************************************
//****
//**** Class ExpansionBuffer: Implementation 
//****
//******************************************************************************

ExpansionBuffer::ExpansionBuffer(UINT8 *buffer, int bufsize) :
    m_compressed(buffer),
    max_compressed_size(bufsize)
{
    Reset();
}

void ExpansionBuffer::Reset()
{
    Rewind();
    memset( m_previous_words, 0, sizeof(m_previous_words) );
    memset( m_compressed, 0, sizeof(m_compressed) );
}

void ExpansionBuffer::Rewind()
{
    m_word_count = 0;
    m_compressed_size = 0;
    m_nochange_count = 0;
    m_bitcount = 0;
    m_bitbuffer = 0;
    m_compressed_size_limit = 0;
    m_crc.Reset();
}

UINT32 ExpansionBuffer::GetBits( INT32 count )
{
    // byte      |2 2 2 2 2 2 2 2|1 1 1 1 1 1 1 1|
    // fragment  |7 7|6 6|5 5|4 4 4 4|3 3|2 2|1 1|                                 

    while(m_bitcount<count)
    {
        assert( m_compressed_size <= m_compressed_size_limit );
        m_bitbuffer |= m_compressed[m_compressed_size++] << m_bitcount;
        m_bitcount += 8;
    }

    UINT32 result = m_bitbuffer & (~0U >> (32-count));
    m_bitbuffer >>= count;
    m_bitcount -= count;
    return result;
}

UINT16 ExpansionBuffer::GetUINT16()
{
    assert( m_word_count < max_word_count );

    UINT16 word=0;

    if( m_nochange_count )
    {
        m_nochange_count--;
        word = m_previous_words[m_word_count];
    }
    else
    {
        UINT32 head = GetBits(2);
        switch(head)
        {
        case 0: 
            // 00 NoChange 1x16 bit
            word = m_previous_words[m_word_count];
            break;

        case 1:
            // 01 00 NoChange 2x16 bit
            // 01 01 NoChange 3x16 bit
            // 01 10 NoChange 4x16 bit
            // 01 11 xxxx NoChange 5..19x16 bit
            // 01 11 1111 xxxxxxxx NoChange 20..274 x16 bit
            // 01 11 1111 11111111 xxxxxxxx-xxxxxxxx NoChange 275... x16 bit
            word = m_previous_words[m_word_count];

            {
                UINT32 count = GetBits(2);
                if( count<3 )
                {
                    m_nochange_count = count+2-1;
                }
                else
                {
                    count = GetBits(4);
                    if( count<15 )
                    {
                        m_nochange_count = count+5-1;
                    }
                    else
                    {
                        count = GetBits(8);

                        if( count<255 )
                        {
                            m_nochange_count = count+20-1;
                        }
                        else
                        {
                            count = GetBits(16);
                            m_nochange_count = count+275-1;
                        }
                    }
                }
            }
            break;

        case 2:
            // 10 Toggle (0 to 1, everything else to 0
            word = m_previous_words[m_word_count] ? 0 : 1;
            break;

        case 3:
            // 11 16 bit follow immediately
            word = (UINT16) GetBits(16);
            break;
        }
    }

    m_previous_words[m_word_count++]=word;
    m_crc.Add16bit(word);
    return word;
}
