/************************************************************************/
/* <By Jackie 2013-7-25>
����������ʽ�Ľ�����, ������һ��NFA�Զ���, �ܴ��������������ʽƥ��
/************************************************************************/


#ifndef __NFA_H__
#define __NFA_H__

namespace RegexCpp
{
	typedef struct State
	{
		int code;		//״̬��, һ�����Զ����б��ϵ��ַ�, ���������S(��ʼ), T(����), FAKE(α�ڵ�,������ת�Ƶı�)
		State* out;		//����1
		State* out1;	//����2
		int lastList;	//��¼��һ�����ڵ�����, �����õ�.
		int id;			// <By�ŷ�ѩ 2013-5-27>���id������ʾ, ��ʾ���ĸ��ڵ�.

		void Init(int code, State* out, State* out1)
		{
			this->code = code;
			this->out = out;
			this->out1 = out1;
			this->lastList = -1;
		}

		void Init()
		{
			code = FAKE;
			out = out1 = NULL;
			lastList = -1;
		}
		void Init(int code)
		{
			this->code = code;
			out = out1 = NULL;
			lastList = -1;
		}

	}State;

	typedef struct Fragment
	{
		State* start; //��NFA�Ľ���ڵ�
		State* end;   //��NFA�ĳ��ڵ�

		void Init()
		{
			start = end = NULL;
		}
		void Init(State* start, State* end)
		{
			this->start = start;
			this->end = end;
		}
	}Fragment;

	typedef struct StateList
	{
		State* list[MAXN];
		int nStates;
	}StateList;

	class Automata
	{
	public:
		Automata();
		~Automata();
		Fragment* CreateAutomataWithFakeStates(Node* root); //�ӱ��ʽ��ת����NFA��ͼ, ͼֻ��һ�����, ������һ��State* ��ʾ����
		State* CreateAutomata(Node* root);
		bool Match(State* start, char* text);
		void Dump(State* state);
		void Dump(bool showFakeStates = true);
		void SetDebug(bool debug){_debug = debug;}

	private:
		State* _state_buffer;
		Fragment* _frag_buffer;
		int _used_state;
		int _used_frag;
		int _list_id;
		bool _debug;
		int _text_start_pos; //By Jackie 2013-12-2  ��¼��ǰ�Ǵ�text��ʲôλ�ÿ�ʼƥ��, ��^����ʹ��
		int _text_end_pos;
		char* _text;
		StateList* _current_list;
		StateList* _next_list;
		State* NewState(){return &_state_buffer[_used_state++];}
		Fragment* NewFragment(){return &_frag_buffer[_used_frag++];}
		void ResetStates(State* start);
		void DumpStateList(StateList* list);
		bool IsMatch(StateList* list);
		void Step(int code);
		void AddState(StateList* list, State* state);
		void PrintCode(int code);

	};
}

#endif