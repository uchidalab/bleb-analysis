/* Main.cpp
 *
 * Copyright (c) 2010, IMURA Masataka.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
 * IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include	<iostream>

#include	"BmpFileIO.h"
#include	"ColorConv.h"
#include	"Labeling.h"

using namespace std;

#define	SRC_IMAGE			"src_sd.bmp"
#define	RESULT_IMAGE		"result.bmp"



// ���ʂ̏o��

void
WriteResult( const char *filename, int w, int h, short *result )
{
	unsigned char	*result_image = new unsigned char[ w * h * 4 ];
	ColorConv<float>	c;

	for ( int i = 0; i < w * h; i++ ) {
		float	r, g, b;
		if ( result[ i ] ) {
			c.HsvToRgb(( result[ i ] % 16 ) / 16.0f, 0.5f, 1.0f, r, g, b );
		} else {
			r = g = b = 0.0f;
		}
		result_image[ i * 4 ] = static_cast<unsigned char>( r * 255.0f );
		result_image[ i * 4 + 1 ] = static_cast<unsigned char>( g * 255.0f );
		result_image[ i * 4 + 2 ] = static_cast<unsigned char>( b * 255.0f );
		result_image[ i * 4 + 3 ] = 255;
	}

	BmpFileIO	b;
	b.WriteRGBA( filename, w, h, result_image );

	delete [] result_image;
}

int
main( int argc, char **argv )
{
	BmpFileIO	b;
	int	w, h;

	// �t�@�C������摜�ǂݍ��� ///////////////////////////////////////////////

	unsigned char	*src_image;
	src_image = b.ReadRGBA( SRC_IMAGE, w, h );
	cout << "image size: " << w << "," << h << endl;

	// �摜�o�b�t�@�m�� ///////////////////////////////////////////////////////
	
	// ����: unsigned char x 1
	// �o��: short x 1

	unsigned char	*src = new unsigned char[ w * h ];
	short	*result = new short[ w * h ];

	// �O���C�X�P�[���ɕϊ� ///////////////////////////////////////////////////
	
	for ( int i = 0; i < w * h; i++ ) {
		src[ i ] = static_cast<unsigned char>( 0.299 * src_image[ i * 4 ]
											   + 0.587 * src_image[ i * 4 + 1 ]
											   + 0.114 * src_image[ i * 4 + 2 ]
			);	// RGB����YCbCr��Y���v�Z
	}

	// �A���̈撊�o�̎��s /////////////////////////////////////////////////////

	LabelingBS	labeling;
	labeling.Exec( src, result, w, h, true, 0 );
	
	// �����͏���
	// ���͉摜�̐擪�A�h���X(unsigned char *)
	// �o�͉摜�̐擪�A�h���X(short *)
	// �摜�̕�(int)
	// �摜�̍���(int)
	// �̈�̑傫�����Ƀ\�[�g���邩(bool) - true:���� false:���Ȃ�
	// �������鏬�̈�̍ő�T�C�Y(int) - ����ȉ��̃T�C�Y�̗̈����������

	// �������ʂ� result (���w���摜�o�b�t�@)�Ɋi�[�����B
	
	// �܂��A�e�A���̈�̏��� labeling ���ɕێ�����Ă���A
	// ���\�b�h��ʂ��Ď��o�����Ƃ��ł���B���̏���
	// ���̉摜�̃��x�����O�����s����ƁA���������B

	// ���ʂ̉摜�t�@�C���ւ̏o�� /////////////////////////////////////////////
#if 1
	WriteResult( RESULT_IMAGE, w, h, result );
#endif
	// �A���̈���̎��o���Ɖ�ʂւ̏o�� ///////////////////////////////////

	int	num_result_regions = labeling.GetNumOfResultRegions();
	cout << "number of result regions: " << num_result_regions << endl;
#if 1	
	for ( int i = 0; i < num_result_regions; i++ ) {
		RegionInfoBS	*ri;
		ri = labeling.GetResultRegionInfo( i );
		cout << "region " << i << "----------------" << endl;
		cout << *ri << endl;
	}
#endif
	// �o�b�t�@�̉�� /////////////////////////////////////////////////////////
	
	delete [] src_image;
	delete [] src;
	delete [] result;

	return 0;
}
