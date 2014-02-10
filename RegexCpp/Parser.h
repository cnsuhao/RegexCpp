/************************************************************************
<By Jackie 2013-7-25>
�﷨����ʵ���߼�, ��Ҫ��;, 1, Ϊ����NFA�ṩ�﷨������, 2, ���pattern�﷨�Ƿ���ȷ
/************************************************************************/

#ifndef __PARSER_H__
#define __PARSER_H__

#include <cstdlib>
#include <ctype.h>
#include <cstring>
#include "Symbol.h"

using namespace std;

#define MAXN 1000000

namespace RegexCpp
{

	typedef struct Node
	{
		TokenType token;
		int subType;			//<By�ŷ�ѩ 2013-5-18>���ڴ���һ��token���Ӧ���ֵ�����.
		Node* left;
		Node* right;

		void Init(TokenType tok, int subT, Node* l, Node* r)
		{
			token = tok;
			subType = subT;
			left = l;
			right = r;
		}
	}Node;


	class Parser
	{
	public:
		Parser();
		~Parser();
		Node* Parse(char* regexStr);
		void Dump(Node* root);
		void SetDebug(bool debug){_debug = debug;}
		bool GetDebug(){return _debug;}

	private:
		Node* _buffer;
		int _used;
		int _pos;
		char* _regex;
		int _len;
		bool _debug;
		bool _error;

		Node* NewNode()
		{
			return &_buffer[_used++];
		}
		char CurrentToken()
		{
			if(_pos >= _len) return -1;
			return _regex[_pos];
		}
		void NextToken()
		{
			_pos++;
		}

		bool Validate(); //����������ʽ����ȷ�ԺϷ��Խ���У��, ���������Ƿ�ƥ��

		Node* Parse();				// recursive implementation
		Node* ParseAlt();			// ����|��
		Node* ParseConcat();		// ���ӷ��Ž���, ���ӷ�������ʽ��
		Node* ParsePlus();			// + ���Ž���
		Node* ParseBracket();		//��������
		Node* ParseDot();			//����.��
		Node* ParseAtom();			//����һ��Ԫ��
		Node* ParseMulti();			//����*��
		Node* ParseSlash();			//����\����, ֧��\d\D\w\W
		
	};
}

#endif // __PARSER_H__
