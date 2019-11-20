// ConsoleApplication1.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include "pch.h"
#include <vector>
#include <functional>
#include <iostream>
#include <string>
#include <sstream>
#include <cassert>

/// Parser for the Akuna Lego Unit input file
//
// When the process() method is called, this class read from 
// stdin and call callbacks for each unit, input, connection, and
// value.
class input_parser {
public:
	/// Register the callback for each UNIT in the input file.
	/// The first parameter is the "unit-name", the second parameter is the "unit-type".
	/// E.g:
	///    a := sum
	/// We would call unit_callback_("a", "sum")
	///
	void register_unit_callback(std::function<void(std::string, std::string, void* p)> cb)
	{
		unit_callback_ = cb;
	}

	/// Register the callback for the INPUTS line in the input file
	///
	/// The parameter tells how many input ports there are.
	/// E.g:
	///    INPUTS: 2
	/// We would call input_callback_(2)
	void register_input_callback(std::function<void(int, void*p)> cb)
	{
		input_callback_ = cb;
	}

	/// Register the callback for each CONNECTION in the input file
	///
	/// The first 3 parameters tell you the "from" side,
	/// the last 3 parameters tell you the "to" side.
	///
	/// For each three parameter group:
	///  - The first is the unit name, "input" or "result"
	///  - The second is "in", "out" or "" (empty)
	///  - The third is port number (could be empty string), e.g. "0", "1", or "".
	/// Example 1:
	///   input/0 -- a/in/0
	///   =>   connection_callback_("input", "", "0", "a", "in", "0")
	/// Example 2:
	///   a/out/0 -- result
	///   =>   connection_callback_("a", "out", 0, "result", "", "")
	/// Example 3:
	///   a/out/3 -- b/in/1
	///   =>   connection_callback_("a", "out", "3", "b", "in", "1")
	/// 
	void register_connection_callback(std::function<void(std::string, std::string, std::string, std::string, std::string, std::string, void*p)> cb)
	{
		connection_callback_ = cb;
	}

	/// Register the callback for each VALUE in the input file
	/// 
	/// The first parameter is always "input" :)
	/// The second parameter is the input port number in string. e.g. "0", "1"
	/// The third parameter is the value number in string. e.g. "999"
	/// E.g:
	///    input/0 := 3
	/// We would call value_callback_("input", "0", "3")
	void register_value_callback(std::function<void(std::string, std::string, std::string, void*p)> cb)
	{
		value_callback_ = cb;
	}

	/// Process lines from stdin and call the appropriate callbacks.  Note that 
	/// this is very intolerant of syntax errors, and will assert in any unexpected
	/// input
	void process()
	{
		// process the UNITS line
		int units_num = get_section_title_line();

		// handle the expected number of units
		for (int i = 0; i < units_num; i++)
		{
			std::string name, type;
			get_section_internal_line(name, type);
			if (unit_callback_)
				unit_callback_(name, type, m_p);
		}

		// process the INPUTS line
		int inputs_num = get_section_title_line();
		if (input_callback_)
			input_callback_(inputs_num, m_p);

		// process the CONNECTIONS line
		int connections_num = get_section_title_line();

		// handle the expected number of connections
		for (int i = 0; i < connections_num; i++)
		{
			std::string from, to;
			get_section_internal_line(from, to);
			assert(from != "result");
			std::vector<std::string> from_strs = split(from, '/');
			assert(from_strs.size() >= 2);
			std::vector<std::string> to_strs = split(to, '/');
			assert(from_strs.size() >= 1);

			// the first parameter is from_strs[0]
			std::string second = from_strs[0] == "input" ? "" : from_strs[1];
			std::string third = from_strs[0] == "input" ? from_strs[1] : from_strs[2];

			std::string fifth = "";
			std::string sixth = "";
			if (to_strs[0] == "input")
			{
				sixth = to_strs[1];
			}
			else if (to_strs[0] != "result")
			{
				fifth = to_strs[1];
				sixth = to_strs[2];
			}

			if (connection_callback_)
				connection_callback_(from_strs[0], second, third,
					to_strs[0], fifth, sixth, m_p);
		}

		// process the VALUES line
		int values_num = get_section_title_line();

		// handle the expected number of values
		for (int i = 0; i < values_num; i++)
		{
			std::string input, value;
			get_section_internal_line(input, value);
			std::vector<std::string> strs = split(input, '/');
			assert(strs.size() == 2);
			if (value_callback_)
				value_callback_(strs[0], strs[1], value, m_p);
		}
	}

	void setDataContainer(void* p)
	{
		m_p = p;
	}

private:
	// hackerrank doesn't support boost files, so use our own utilities...

	// split a string on a delimiter and append each item to result. 
	template<typename Out>
	void split(const std::string &s, char delim, Out result)
	{
		std::stringstream ss;
		ss.str(s);
		std::string item;
		while (std::getline(ss, item, delim))
		{
			*(result++) = item;
		}
	}

	// split a string an a delimiter and return a vector of the results
	std::vector<std::string> split(const std::string &s, char delim)
	{
		std::vector<std::string> elems;
		split(s, delim, std::back_inserter(elems));
		return elems;
	}

	// read a line from stdin and return the number in the second token
	int get_section_title_line()
	{
		std::string line;
		std::getline(std::cin, line);
		std::vector<std::string> words = split(line, ' ');
		assert(words.size() == 2);
		return atoi(words[1].c_str());
	}

	// read a line from stdin and return the first and third token
	void get_section_internal_line(std::string& a, std::string& b)
	{
		std::string line;
		std::getline(std::cin, line);
		std::vector<std::string> words = split(line, ' ');
		assert(words.size() == 3);
		a = words[0];
		b = words[2];
	}

private:
	void* m_p;

	std::function<void(std::string, std::string, void*p)> unit_callback_;

	std::function<void(int n, void*p)> input_callback_;

	std::function<void(std::string, std::string, std::string, std::string, std::string, std::string, void*p)> connection_callback_;

	std::function<void(std::string, std::string, std::string, void*p)> value_callback_;
};


#include <vector>
#include <map>
#include <Windows.h>

//store the units information
typedef struct {
	std::string unit_name; //unit name
	std::string unit_type; //unit type
}Unit, *pUnit;

//instruction
struct Instruction {
	std::string str1;
	std::string str2;
	std::string str3;

	friend bool operator==(const Instruction& l, const Instruction& r)
	{
		if (l.str1 == r.str1 && l.str2 == r.str2 && l.str3 == r.str3)
		{
			return true;
		}
		else
		{
			return false;
		}
	}

	friend bool operator<(const Instruction& l, const Instruction& r)
	{
		std::string ltemp = l.str1 + l.str2 + l.str3;
		std::string rtemp = r.str1 + r.str2 + r.str3;
		if (ltemp < rtemp)
		{
			return true;
		}
		else
		{
			return false;
		}
	}
};

//instruction connection
typedef struct {
	Instruction From;
	Instruction To;
}Connection;

//value type
typedef struct {
	Instruction From;
	int value;
}Value;


void unit_callback(std::string name, std::string type, void* p);
void input_callback(int n, void* p);
void connection_callback(std::string from1, std::string from2, std::string from3, std::string to1, std::string to2, std::string to3, void* p);
void value_callback(std::string input, std::string number, std::string value, void* p);

//get the data from input parser
class input_get
{
public:
	friend void unit_callback(std::string name, std::string type, void* p)
	{
		Unit unit;
		unit.unit_name = name;
		unit.unit_type = type;
		((input_get *)p)->m_units.push_back(unit);
	}

	friend void input_callback(int n, void* p)
	{
		((input_get *)p)->inputs = n;
	}

	friend void connection_callback(std::string from1, std::string from2, std::string from3, std::string to1, std::string to2, std::string to3, void* p)
	{
		Instruction from;
		from.str1 = from1;
		from.str2 = from2;
		from.str3 = from3;

		Instruction to;
		to.str1 = to1;
		to.str2 = to2;
		to.str3 = to3;

		Connection conn;
		conn.From = from;
		conn.To = to;
		((input_get *)p)->m_conns.push_back(conn);
	}

	friend void value_callback(std::string input, std::string number, std::string value, void* p)
	{
		Instruction from;
		from.str1 = input;
		from.str2 = "";
		from.str3 = number;

		Value v;
		v.From = from;
		v.value = std::atoi(value.c_str());

		((input_get *)p)->m_values.push_back(v);
	}

	std::vector<Unit>* getUnits()
	{
		return &m_units;
	}

	int* getInputs()
	{
		return &inputs;
	}

	std::vector<Connection> getConnections()
	{
		return m_conns;
	}

	std::vector<Value> getValues()
	{
		return m_values;
	}

private:
	std::vector<Unit> m_units;
	int inputs;
	std::vector<Connection> m_conns;
	std::vector<Value> m_values;
};


/// calculator class
class calculaotr {
public:
	calculaotr()
	{
		m_support_types.push_back("sum");
		m_support_types.push_back("negate");
		m_support_types.push_back("max");
		m_support_types.push_back("min");
		m_support_types.push_back("mul");
	}

	/// Set the calculation type
	/// 
	/// The first parameter is type "sum" etc
	/// Return if supported then return succeeded
	bool setType(std::string type)
	{
		bool supportFlag = false;
		for (size_t i = 0; i < m_support_types.size(); i++)
		{
			if (m_support_types[i] == type)
			{
				supportFlag = true;
				break;
			}
		}

		m_unit_type = type;

		return supportFlag;
	}

	/// Run calculation
	/// 
	/// The first parameter is data list
	/// Return the result, only support int
	int run(std::vector<int> ops)
	{
		//std::cout<<m_unit_type<<std::endl;
		long ret = 0;
		if (m_unit_type == "sum")
		{
			for (size_t i = 0; i < ops.size(); i++)
			{
				//std::cout<<ops[i]<<std::endl;
				ret += (long)ops[i];
			}
		}
		else if (m_unit_type == "negate")
		{
			ret = -1 * ops[0];
		}
		else if (m_unit_type == "max")
		{
			for (size_t i = 0; i < ops.size(); i++)
			{
				if (ret < ops[i])
				{
					ret = ops[i];
				}
			}
		}
		else if (m_unit_type == "min")
		{
			for (size_t i = 0; i < ops.size(); i++)
			{
				if (ret > ops[i])
				{
					ret = ops[i];
				}
			}
		}
		else if (m_unit_type == "mul")
		{
			for (size_t i = 0; i < ops.size(); i++)
			{
				ret *= ops[i];
			}
		}

		return ret;
	}

private:
	std::string m_unit_type;
	std::vector<std::string> m_support_types;
};

//define the link list of logic units
struct logic_unit {
	std::string unit_name;
	std::string unit_type;
	std::vector<Instruction> inputs;
	std::vector<bool> data_flag;
	std::vector<int> data_array;
	std::vector<logic_unit*> pUnit_array;
};

//define the input processor class
class input_process {
public:
	input_process()
	{
		m_pUnit = new logic_unit();
		m_pUnit->unit_name = "root";
		m_unit_names.push_back(m_pUnit->unit_name);
	}

	~input_process()
	{
		deleteUnit(m_pUnit);
	}

	void deleteUnit(logic_unit* _pUnit)
	{
		for (size_t i = 0; i < _pUnit->pUnit_array.size(); i++)
		{
			deleteUnit(_pUnit->pUnit_array[i]);
		}

		delete _pUnit;
	}

	bool isUnitNameExists(std::string _unitName)
	{
		for (size_t i = 0; i < m_unit_names.size(); i++)
		{
			if (m_unit_names[i] == _unitName) {
				return true;
			}
		}

		return false;
	}

	void getUnit(std::string _unitName, bool& _rf, logic_unit*& _rUnit)
	{
		getUnitR(_unitName, m_pUnit, _rf, _rUnit);
	}

	void getUnitR(std::string _unitName, logic_unit* _pUnit, bool& _rf, logic_unit*& _rUnit)
	{
		if (_pUnit->unit_name == _unitName)
		{
			_rUnit = _pUnit;
			_rf = true;
		}
		else
		{
			for (size_t i = 0; i < _pUnit->pUnit_array.size(); i++)
			{
				getUnitR(_unitName, _pUnit->pUnit_array[i], _rf, _rUnit);

				if (_rf)
				{
					break;
				}
			}
		}
	}

	void addUnit(std::string _unitName, logic_unit* _pCUnit)
	{
		bool rf = false;
		logic_unit* cur = NULL;
		getUnit(_unitName, rf, cur);
		if (cur != NULL)
		{
			cur->pUnit_array.push_back(_pCUnit);
			m_unit_names.push_back(_pCUnit->unit_name);
		}
	}

	void assignInputValues(Instruction _input, int _value)
	{
		std::map<Instruction, std::vector<std::string>>::iterator it = m_input_to_unit_name.find(_input);
		if (it == m_input_to_unit_name.end())
		{
			return;
		}

		//iterator units
		for (size_t i = 0; i < it->second.size(); i++)
		{
			bool rf = false;
			logic_unit* pCurrUnit = NULL;
			getUnit(it->second[i], rf, pCurrUnit);
			if (rf && pCurrUnit != NULL)
			{
				for (size_t j = 0; j < pCurrUnit->inputs.size(); j++)
				{
					if (pCurrUnit->inputs[j] == _input)
					{
						pCurrUnit->data_flag[j] = true;
						pCurrUnit->data_array[j] = _value;
					}
				}
			}
		}
	}

	void getUnitResult(logic_unit* _pUnit, bool& _rf, int& _r)
	{
		if (_pUnit->unit_name == "root")
		{
			for (size_t i = 0; i < m_pUnit->pUnit_array.size(); i++)
			{
				getUnitResult(m_pUnit->pUnit_array[i], _rf, _r);
				if (!_rf)
				{
					break;
				}
			}
		}
		else if (_pUnit->unit_name == "result")
		{
			if (_pUnit->data_flag[0] == true)
			{
				_r = _pUnit->data_array[0];
				_rf = true;
			}
			else
			{
				_rf = false;
			}
		}
		else
		{
			calculaotr c;

			if (!c.setType(_pUnit->unit_type))
			{
				_rf = false;
			}

			bool dataReadyFlag = true;
			for (size_t i = 0; i < _pUnit->data_flag.size(); i++)
			{
				if (_pUnit->data_flag[i] == false)
				{
					dataReadyFlag = false;
					break;
				}
			}

			if (dataReadyFlag)
			{
				int result = c.run(_pUnit->data_array);
				//Need to assign result to the units which needs it
				Instruction unitOutput;
				unitOutput.str1 = _pUnit->unit_name;
				unitOutput.str2 = "out";
				unitOutput.str3 = "0"; //so only support one output here.
				assignInputValues(unitOutput, result);
				_rf = true;

				for (size_t i = 0; i < _pUnit->pUnit_array.size(); i++)
				{
					getUnitResult(_pUnit->pUnit_array[i], _rf, _r);
					if (!_rf)
					{
						break;
					}
				}
			}
			else
			{
				_rf = false;
			}
		}
	}

	void getFinalResult(bool& _rf, int& _r)
	{
		getUnitResult(m_pUnit, _rf, _r);
	}

	void addInputMap(Instruction _od, std::string _unit_name)
	{
		//std::cout << "addInputMap" << std::endl;
		bool keyFound = false;
		std::map<Instruction, std::vector<std::string>>::iterator it = m_input_to_unit_name.begin();
		for (; it != m_input_to_unit_name.end(); ++it)
		{
			if (it->first == _od)
			{
				bool unitNameFound = false;
				for (size_t i = 0; i < it->second.size(); i++)
				{
					if (it->second[i] == _unit_name)
					{
						unitNameFound = true;
					}
				}

				if (!unitNameFound)
				{
					it->second.push_back(_unit_name);
				}

				keyFound = true;
			}
		}

		if (!keyFound)
		{
			std::vector<std::string> v;
			v.push_back(_unit_name);
			m_input_to_unit_name.insert(std::pair<Instruction, std::vector<std::string>>(_od, v));
		}
	}

	void printInputUnitNameMap()
	{
		int index = 1;
		std::map<Instruction, std::vector<std::string>>::iterator it = m_input_to_unit_name.begin();
		for (; it != m_input_to_unit_name.end(); ++it)
		{
			std::cout << index++ << std::endl;
			std::cout << it->first.str1 << std::endl;
			std::cout << it->first.str2 << std::endl;
			std::cout << it->first.str3 << std::endl;

			for (size_t i = 0; i < it->second.size(); i++)
			{
				std::cout << it->second[i] << std::endl;
			}

			std::cout << "Finished" << std::endl;
		}
	}

	std::string getUnitTypeByName(std::string _unitName, std::vector<Unit> _units)
	{
		for (size_t i = 0; i < _units.size(); i++)
		{
			if (_units[i].unit_name == _unitName)
			{
				return _units[i].unit_type;
			}
		}

		return "";
	}

private:
	std::vector<std::string> m_unit_names;
	std::map<Instruction, std::vector<std::string>> m_input_to_unit_name;
	logic_unit* m_pUnit;
};

int main()
{
	//get the values
	input_parser ip;
	input_get ig;

	ip.setDataContainer(&ig);
	ip.register_unit_callback(unit_callback);
	ip.register_input_callback(input_callback);
	ip.register_connection_callback(connection_callback);
	ip.register_value_callback(value_callback);
	ip.process();


	//create the tree
	input_process pp;
	std::vector<Connection> conns = ig.getConnections();

	for (size_t i = 0; i < conns.size(); i++)
	{
		if (!pp.isUnitNameExists(conns[i].To.str1))
		{
			logic_unit* newUnit = new logic_unit();
			newUnit->unit_name = conns[i].To.str1;
			newUnit->unit_type = pp.getUnitTypeByName(conns[i].To.str1, *ig.getUnits());
			newUnit->inputs.push_back(conns[i].From);
			newUnit->data_flag.push_back(false);
			newUnit->data_array.push_back(0); //init the inputs value
			if (conns[i].From.str1 == "input")
			{
				pp.addUnit("root", newUnit);
			}
			else
			{
				pp.addUnit(conns[i].From.str1, newUnit);
			}

			pp.addInputMap(conns[i].From, conns[i].To.str1);
		}
		else
		{
			bool resultFlag = false;
			logic_unit* pUnit = NULL;
			pp.getUnit(conns[i].To.str1, resultFlag, pUnit);
			if (resultFlag)
			{
				bool found = false;
				for (size_t j = 0; i < pUnit->inputs.size(); j++)
				{
					if (pUnit->inputs[i] == conns[i].From)
					{
						found = true;
					}
				}

				if (!found)
				{
					pUnit->inputs.push_back(conns[i].From);
					pUnit->data_flag.push_back(false);
					pUnit->data_array.push_back(0); //init the inputs value
					pp.addInputMap(conns[i].From, conns[i].To.str1);
				}
			}
		}
	}

	std::vector<Value> values = ig.getValues();
	//calculator the result via tree
	for (size_t i = 0; i < values.size(); i++)
	{
		pp.assignInputValues(values[i].From, values[i].value);

		//DWORD start = GetTickCount();
		bool resultFlag = false;
		int result = 0;
		pp.getFinalResult(resultFlag, result);
		if (resultFlag)
		{
			std::cout << result /*<<"\t" << GetTickCount()-start*/ << std::endl;
		}
	}

	return 0;
}
