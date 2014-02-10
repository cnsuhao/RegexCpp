#include "StdAfx.h"
#include <Windows.h>
#include <iostream>
#include <cassert>
#include "Symbol.h"
#include "Parser.h"
#include "Automata.h"
#include "ConsoleColor.h"


using namespace std;

namespace RegexCpp
{

	Automata::Automata()
	{
		_state_buffer = new State[MAXN];
		_frag_buffer = new Fragment[MAXN];
		_used_state = 0;
		_used_frag = 0;
		_current_list = new StateList();
		_next_list = new StateList();
		_list_id = 0;
		_debug = false;
	}

	Automata::~Automata()
	{
		delete []_state_buffer;
		delete []_frag_buffer;
		delete _current_list;
		delete _next_list;
	}

	//<By�ŷ�ѩ 2013-5-20> �����NFA, ��ʹ�����FAKE��State�ڵ�, Ϊ�˷���, ʹ��һ��Fragment ���һ��һ����.
	//Ŀǰfake�ڵ㻹���ܱ�clear��, ���������������.

	Fragment* Automata::CreateAutomataWithFakeStates(Node* root)
	{
		switch(root->token)
		{
		case CONCAT:
			{
				Fragment* pre = CreateAutomataWithFakeStates(root->left);
				assert(!(pre->end->out != NULL && pre->end->out1 != NULL));
				Fragment* post = CreateAutomataWithFakeStates(root->right);
				if(pre->end->out == NULL)
					pre->end->out = post->start;
				else 
					pre->end->out1 = post->start;
				Fragment* frag = NewFragment();
				frag->start = pre->start;
				frag->end = post->end;
				return frag;
				break;
			}
		case ALT: //"|" �ڵ�, ��Ҫ����Fake State.
			{
				Fragment* up = CreateAutomataWithFakeStates(root->left);
				Fragment* down = CreateAutomataWithFakeStates(root->right);
				Fragment* frag = NewFragment();
				frag->start = NewState();
				frag->start->Init();
				frag->end = NewState();
				frag->end->Init();
				frag->start->out = up->start;
				frag->start->out1 = down->start;

				assert(!(up->end->out != NULL && up->end->out1 != NULL));
				assert(!(down->end->out != NULL && down->end->out1 != NULL));
				if(up->end->out ==NULL)
					up->end->out = frag->end;
				else 
					up->end->out1 = frag->end;
				if(down->end->out == NULL)
					down->end->out = frag->end;
				else 
					down->end->out1 = frag->end;
				return frag;
			}
		case QUESTION:
			{
				Fragment* core = CreateAutomataWithFakeStates(root->left);
				Fragment* frag = NewFragment();
				frag->start = NewState();
				frag->start->Init();
				frag->end = NewState();
				frag->end->Init();
				frag->start->out = core->start;
				frag->start->out1 = frag->end;
				assert(!(core->end->out != NULL && core->end->out1 != NULL));
				if(core->end->out == NULL)
					core->end->out = frag->end;
				else  
					core->end->out1 = frag->end;
				return frag;
				break;
			}
		case STAR:
			{
				Fragment* core = CreateAutomataWithFakeStates(root->left);
				Fragment* frag = NewFragment();
				frag->start = frag->end = NewState(); //��֤��*����.
				frag->start->Init();
				frag->start->out = core->start;
				assert(!(core->end->out != NULL && core->end->out1 != NULL));
				if(core->end->out == NULL)
					core->end->out = frag->start;
				else 
					core->end->out1 = frag->start;
				return frag;
				break;
			}
		case PLUS:
			{
				Fragment* core = CreateAutomataWithFakeStates(root->left);
				Fragment* frag = NewFragment();
				frag->start = core->start;
				frag->end = NewState();
				frag->end->Init();
				frag->end->out = frag->start;
				frag->end->out1 = NULL;

				assert(!(core->end->out != NULL && core->end->out1 != NULL));
				if(core->end->out == NULL)
					core->end->out = frag->end;
				else 
					core->end->out1 = frag->end;
				return frag;
				break;
			}
		case NUM: 
		case ALPHA:
			{
				Fragment* frag = NewFragment();
				frag->Init();
				State* core = NewState();
				core->Init(root->subType);
				frag->start = core;
				frag->end = core;
				return frag;
				break;
			}
		case DOT:
		case ALL_NUM:
		case NON_NUM:
		case ALL_ALNUM:
		case NON_ALNUM:
		case START:
		case END:
			{
				Fragment* frag = NewFragment();
				frag->Init();
				State* core = NewState();
				core->Init(root->token);
				frag->start = core;
				frag->end = core;
				return frag;
			}
		case TRIVIAL:
			{
				Fragment* frag = NewFragment();
				frag->Init();
				State* core = NewState();
				core->Init(root->subType);
				frag->start = core;
				frag->end = core;
				return frag;
				break;
			}
		}

		return NULL;
	}


	//By Jackie 2013-12-1 �������.
	State* Automata::CreateAutomata(Node* root) 
	{
		_used_frag = 0;
		_used_state = 0;
		Fragment* frag = CreateAutomataWithFakeStates(root);
		State* start = NewState();
		start->Init(S);
		State* end = NewState();
		end->Init(T);
		start->out = frag->start;
		if(frag->end->out == NULL)
			frag->end->out = end;
		else 
			frag->end->out1 = end;
		
		if(_debug) 
		{
			cout << "===================== Dump the automata with fake states =====================" << endl;
			Dump();
			cout << "==============================================================================" << endl;
		}
		return start;
	}

	bool Automata::IsMatch(StateList* list)
	{
		for(int i = 0; i < list->nStates; i++)
		{
			if(list->list[i]->code == T)
				return true;
		}
		return false;
	}

	//By Jackie 2014-1-2 ��Step������, ���п���һ��״̬�ĺ��״̬�ܲ��ܽ�����һ�������
	void Automata::Step(int code)
	{
		_next_list->nStates = 0;
		_list_id++;

		for(int i = 0; i < _current_list->nStates; i++)
		{
			State* state = _current_list->list[i];
			bool bingo = false;
			if(state->code == code) bingo = true;
			else if(state->code == DOT) bingo = true;
			else if(state->code == ALL_NUM && code >= '0' && code <= '9') bingo = true;
			else if(state->code == ALL_ALNUM && isalnum(code)) bingo = true;
			else if(state->code == NON_NUM && !(code >= '0' && code <= '9')) bingo = true;
			else if(state->code == NON_ALNUM && !isalnum(code)) bingo = true;
			if(bingo)
			{
				AddState(_next_list, state->out);
				AddState(_next_list, state->out1);
			}
		}
	}

	//<By�ŷ�ѩ 2013-5-27>����ݹ�����״̬, ��֤ÿ�ζ������Ч״̬�ӽ���.
	//ռλ�����ܽ���״̬����, ֱ�ӰѺ��״̬�Ž���
	void Automata::AddState(StateList* list, State* state)
	{
		if(state == NULL || state->lastList == _list_id)
		{
			return;
		}
		
		state->lastList = _list_id;
		if(state->code == START && _text_start_pos != 0) return;
		if(state->code == END && _text_end_pos != (strlen(_text)) - 1) return;

		if(state->code == FAKE || state->code == START || state->code == END)
		{
			AddState(list, state->out);
			AddState(list, state->out1);
			return ;
		}
		list->list[list->nStates++] = state	;
	}

	void Automata::ResetStates(State* start)
	{
		if(start->lastList == -1) return;
		start->lastList = -1;
		if(start->out != NULL) ResetStates(start->out);
		if(start->out1 != NULL) ResetStates(start->out1);
	}

	bool Automata::Match(State* start, char* text)
	{
		int len = strlen(text);
		_text = text;
		bool bingo = false;
		for(int i = 0; i < len; i++)
		{
			_list_id = 0;
			_current_list->nStates = 0;
			_next_list->nStates = 0;
			start->lastList = 0;
			_text_start_pos = i;
			_text_end_pos = i;
			ResetStates(start); //ÿ�ΰѿ�ʼ�α�����ƶ�, Ȼ�����½���һ��ƥ�����
			if(start->out != NULL)
				AddState(_current_list, start->out);
			if(start->out1 != NULL)
				AddState(_current_list, start->out1);
			StateList* t = NULL;
			int last = -1;
			
			for(int j = i; j < len; j++)
			{
				int code = text[j];
				_text_end_pos = j;
				Step(code);
				t = _current_list;
				_current_list = _next_list;
				_next_list = t;
				if(IsMatch(_current_list)) 
				{
					last = max(last, j);
				}
			}
			if(last != -1)
			{
				cout << green << "Match: ";
				for(int k = i; k <= last; k++) cout << text[k];
				cout << " [" << i << "," << last << "]";
				cout << flushcolor;
				bingo = true;
				i = last;
			}
		}
		
		if(!bingo) 
		{
			cout << red << "No matched text" << flushcolor;
		}
		return bingo;
	}

	void Automata::DumpStateList(StateList* list)
	{
		cout << "Count(" <<list->nStates << "): ";
		for(int i = 0; i < list->nStates; i++)
		{
			if(list->list[i]->code == FAKE)
			{
				cout << "FAKE ";
			}
			else if(list->list[i]->code == S)
			{
				cout << "S " ;
			}
			else  if(list->list[i]->code == T)
			{
				cout << "T ";
			}
			else if(list->list[i]->code >= 0 && list->list[i]->code <= 15)
			{
				cout << TokenString[list->list[i]->code] << " " ;
			}
			else 
			{
				cout << char(list->list[i]->code) << " ";
			}
		}
		cout << endl;
	}

	void Automata::Dump(State* state)
	{
		if(state == NULL) return;
		cout << state->code;
		if(state->out) cout << " " << state->out->code;
		if(state->out1) cout << " " << state->out1->code;
		cout << endl;
		Dump(state->out);
		Dump(state->out1);

	}
	void Automata::PrintCode(int code)
	{
		if(code == -1) cout << "S" ;
		else if(code == -2) cout << "T";
		else if(code == FAKE) cout << "FAKE";
		else if(code < 0)
		{
			cout << red << "{Unknown state code in automata " << code << "}" << flushcolor;
			return;
		}
		if(code < 0) return;
		if(isalnum(code)) 
		{
			cout << char(code);
			return;
		}
		else if(code <= 15 && code >= 0)
			cout << TokenString[code];
		else cout << red << "{Unknown state code in automata " << code << "}" << endl;
	}
	void Automata::Dump(bool showFakeStates)
	{
		
		for(int i = 0; i < _used_state; i++)
		{
			State* state = &_state_buffer[i];
			if(!showFakeStates && state->code == FAKE) continue;
			cout << i << " {";
			PrintCode(state->code);
			cout << ": ";
			
			if(state->out) 
			{
				cout << " ";
				PrintCode(state->out->code);
			}
			if(state->out1) 
			{
				cout << " ";
				PrintCode(state->out1->code);
			}
			cout << "} ";
			if(state->out) cout << state->out - _state_buffer << " ";
			if(state->out1) cout << state->out1 - _state_buffer;
			
			cout << endl;
		}
	}
}