#ifndef _BASE64_H_
#define _BASE64_H_

#include <malloc.h>


#define ERR_BASE64_BUFFER_TOO_SMALL  -0x0010
#define ERR_BASE64_INVALID_CHARACTER -0x0012

static const unsigned char Base64_Encode_Map[64] = 
{
	'A','B','C','D','E','F','G','H',
	'I','J','K','L','M','N','O','P',
	'Q','R','S','T','U','V','W','X',
	'Y','Z','a','b','c','d','e','f',
	'g','h','i','j','k','l','m','n',
	'o','p','q','r','s','t','u','v',
	'w','x','y','z','0','1','2','3',
	'4','5','6','7','8','9','+','/'
};

static const unsigned char Base_Decode_Map[128] = 
{
	127,127,127,127,127,127,127,127,127,127,
	127,127,127,127,127,127,127,127,127,127,
	127,127,127,127,127,127,127,127,127,127,
	127,127,127,127,127,127,127,127,127,127,
	127,127,127,62, 127,127,127,63, 52, 53,
	54, 55, 56, 57, 58, 59, 60, 61, 127,127,
	127,64, 127,127,127,0,  1,  2,  3,  4,  
	5,  6,  7,  8,  9,  10, 11, 12, 13, 14,
	15, 16, 17, 18, 19, 20, 21, 22, 23, 24,
	25, 127,127,127,127,127,127,26, 27, 28,
	29, 30, 31, 32, 33, 34, 35, 36, 37, 38,
	39, 40, 41, 42, 43, 44, 45, 46, 47, 48,
	49, 50, 51, 127,127,127,127,127
};


class Base64
{
public:
	Base64(){};
	~Base64(){};
	static bool Encrypt(const unsigned char * pSrc, int iSlen, unsigned char * pDst, int *iDlen)
	{
		int iRet = Base64_Encode(pSrc, iSlen, pDst, iDlen);
		return iRet == 0;
	};
	static bool Decrypt(const unsigned char * pSrc, int iSlen, unsigned char * pDst, int *iDlen)
	{
		int iRet = Base64_Decode(pSrc, iSlen, pDst, iDlen);
		return iRet == 0;
	};

private:
	static int Base64_Encode(const unsigned char * src, int iSlen, unsigned char * dst, int *iDlen)
	{
			int i, n;
	int c1,c2,c3;
	unsigned char * p;

	if (iSlen == 0)
	{
		return 0;
	}

	n = (iSlen << 3) / 6;
	switch( (iSlen << 3) - (n * 6))
	{
	case 2: n += 3; break;
	case 3: n += 2; break;
	default:break;
	}
	//int x = *iDlen;
	if (*iDlen < n + 1)
	{
		*iDlen = n + 1;
		return ERR_BASE64_BUFFER_TOO_SMALL;
	}

	n = (iSlen / 3) * 3;

	for(i = 0, p = dst; i< n; i += 3)
	{
		c1 = *src++;
		c2 = *src++;
		c3 = *src++;

		*p++ = Base64_Encode_Map[(c1 >> 2) & 0x3F];
		*p++ = Base64_Encode_Map[(((c1 & 3) << 4) + (c2 >> 4)) & 0x3F];
		*p++ = Base64_Encode_Map[(((c2 & 15) << 2) + (c3 >> 6)) & 0x3F];
		*p++ = Base64_Encode_Map[c3 & 0x3F];
	}

	if (i < iSlen)
	{
		c1 = *src++;
		c2 = ((i+1) < iSlen) ? *src++ : 0;

		*p++ = Base64_Encode_Map[(c1 >> 2) & 0x3F];
		*p++ = Base64_Encode_Map[(((c1 & 3) << 4) + (c2 >> 4)) & 0x3F];

		if ((i+1) < iSlen)
		{
			*p++ = Base64_Encode_Map[((c2 & 15) << 2) & 0x3F];
		}
		else
		{
			*p++ = '=';
		}
		*p++ = '=';
	}

	*iDlen = static_cast<int>(p-dst);
	*p = 0;
	return 0;
	};
	static int Base64_Decode(const unsigned char * src, int iSlen, unsigned char * dst, int *iDlen)
	{
			int i, j, n;
	unsigned long x;
	unsigned char *p;

	for( i = j = n = 0; i< iSlen; i++)
	{
		if ((iSlen - i) >= 2 && *(src+i) == '/r' && *(src+i+1) == '/n')
			continue;

		if (*(src+i) == '/n')
			continue;

		if (*(src+i) == '=' && ++j > 2)
			return ERR_BASE64_INVALID_CHARACTER;

		if (*(src+i) > 127 || Base_Decode_Map[*(src+i)] == 127)
			return ERR_BASE64_INVALID_CHARACTER;

		if (Base_Decode_Map[*(src+i)] < 64 && j != 0)
			return ERR_BASE64_INVALID_CHARACTER;

		n++;
	}

	if (n == 0)
		return 0;

	n = (n*6+7) >> 3;

	if (*iDlen < n)
	{
		*iDlen = n;
		return ERR_BASE64_BUFFER_TOO_SMALL;
	}

	for (j =3, n = x = 0, p= dst; i> 0; i--, src++)
	{
		if (*src == '/r' || *src == '/n')
			continue;

		j -= (Base_Decode_Map[*src] == 64);
		x = (x << 6) | (Base_Decode_Map[*src] & 0x3F);

		if (++n == 4)
		{
			n = 0;
			if (j > 0) *p++ = (unsigned char)(x >> 16);
			if (j > 1) *p++ = (unsigned char)(x >> 8);
			if (j > 2) *p++ = (unsigned char)(x);

		}
	}

	*iDlen = static_cast<int>(p - dst);
	
	return 0;
	};
};

#endif
