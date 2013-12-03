#include "StdAfx.h"
#include "BruteForce.h"


//By Jackie 2013-11-30  ���ƥ�䵽�˾ͷ���1, ����0
//��֧������, ���漰��������, ����Ϊ����ʵ���㷨��һ��demonstration.
//֧�ֵķ�������:
//^ ��ʼ����
//* ����ƥ��0�����
//$ ��������, ����pattern������ƥ�䵽�ı���ĩβ����.
//. ����ƥ������һ������
int match(char* regexp, char* text)
{
	if(regexp[0] == '^') return matchhere(regexp, text)  ;
	do
	{
		if(matchhere(regexp, text))
			return 1;
	}while(*text++ != '\0');

	return 0;
}

int matchhere(char* regexp, char* text)
{
	if(regexp[0] == '\0') return 1;
	if(regexp[1] == '*') return matchstar(regexp[0], regexp + 2, text)       ;
	if(regexp[0] == '$' && regexp[1] == '\0' ) return text[0] == '\0';
	if(text[0] != '\0' && (regexp[0] == '.' || regexp[0] == text[0])) return matchhere(regexp + 1, text + 1);

	return 0;
}

int matchstar(int c, char* regexp, char* text)
{
	do
	{
		if(matchhere(regexp, text)) return 1;

	}   while(*text != '\0' && (*text++ == c || c == '.'));

	return 0;
}

int matchplus(int c, char* regexp, char* text)
{
	while(*text != '\0' && (*text++ == c || c == '.'))
	{
		if(matchhere(regexp, text)) return 1;
	}
	return 0;
}