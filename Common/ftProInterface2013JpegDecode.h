///////////////////////////////////////////////////////////////////////////////
//
// File:    ftProInterface2013JpegDecode.h
//
// Project: ftPro - fischertechnik Control Graphical Programming System
//
// Module:  JPEG decode for images received from the TXT
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
// Copyright (C) 2015
//
///////////////////////////////////////////////////////////////////////////////
//
// TO DO:
//
///////////////////////////////////////////////////////////////////////////////
//
// Usage details for module ftProInterface2013JpegDecode
//
// see also:
//
///////////////////////////////////////////////////////////////////////////////

// Decode a JPEG stream in memory, as received from the TXT
// 
// jpegdata = pointer to JPEG byte stream
// jpegsize = Size of JPEG byte stream
// yuvdata  = Pointer to resulting YUV data, typically YUV422 interleaved
// yuvsize  = Size of the resulting YUV data. For YUV422 this are 2 bytes per pixel
bool ftProJpegDec(const UINT8 *jpegdata, int jpegsize, UINT8 *yuvdata, int yuvsize, size_t *bytes_read);
