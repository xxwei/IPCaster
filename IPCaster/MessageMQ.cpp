#include "stdafx.h"
#include "MessageMQ.h"



int		Message::GetType()
{
	//return nType;
	return msg["type"].asInt();
}
void	Message::SetType(int type)
{
	//nType = type;
	msg["type"] = type;
}
bool	Message::AppendMsg(wstring key, wstring value)
{
	//msglist.insert(pair<wstring, wstring>(key, value));
	string skey = string(G2U(WString2String(key).c_str()));
	string svalue = string(G2U(WString2String(value).c_str())); 
	msg[skey] = svalue;
	return true;
}
wstring	Message::GetMsgValue(wstring key)
{
	//if (msglist.find(key) != msglist.end())
	//{
	//	return msglist.find(key)->second;
	//}
	//return L"";
	string skey = string(G2U(WString2String(key).c_str()));
	return String2WString(U2G(msg[skey].asCString()));
}


string  Message::ToString()
{
	//std::wstringstream msgstr;
	//msgstr << nType;
	//map<wstring, wstring>::iterator item = msglist.begin();
	//for (; item!=msglist.end();item++)
	//{
	//	msgstr << "|" << item->first << "$" << item->second;
	//}
	//return msgstr.str();
	return msg.toStyledString();
}

void SplitString(const std::string& s, std::vector<std::string>& v, const std::string& c)
{
	std::string::size_type pos1, pos2;
	pos2 = s.find(c);
	pos1 = 0;
	while (std::string::npos != pos2)
	{
		v.push_back(s.substr(pos1, pos2 - pos1));

		pos1 = pos2 + c.size();
		pos2 = s.find(c, pos1);
	}
	if (pos1 != s.length())
		v.push_back(s.substr(pos1));
}

bool	Message::PraseMsg(string msgstr)
{

	Json::Reader reader;
	return reader.parse(msgstr, msg);
	/*
	wstring split1 = L"|";
	wstring split2 = L"$";
	std::wstring::size_type pos1, pos2;
	pos2 = msgstr.find(split1);
	pos1 = 0;
	nType = _wtoi(msgstr.substr(pos1, pos2 - pos1).c_str());
	msglist.clear();
	while (std::wstring::npos != pos2)
	{
		pos1 = pos2 + split1.size();
		pos2 = msgstr.find(split1, pos1);
		wstring item = msgstr.substr(pos1, pos2 - pos1);

		pos1 = item.find(split2);
		if (pos1 != std::string::npos)
		{
			wstring key = item.substr(0, pos1);
			wstring value = item.substr(pos1 + split2.size(), item.length());
			msglist.insert(pair<wstring, wstring>(key, value));
		}
		else
		{
			if (msglist.size() == 0)
			{
				return false;
			}
			else
			{
				return true;
			}
		}
	}
	if (pos1 != msgstr.length())
	{
		wstring item = msgstr.substr(pos1);
		wstring key = item.substr(0, pos1);
		wstring value = item.substr(pos1 + split2.size(), item.length());
		msglist.insert(pair<wstring, wstring>(key, value));
	}
	return true;
	*/

}

MessageMQ::MessageMQ()
{
	int major, minor, patch;
	zmq_version(&major, &minor, &patch);
}


MessageMQ::~MessageMQ()
{

}

Message * MessageMQ::GetNewMsg()
{
	if (m_list.size() > 0)
	{
		Message *p = (*(m_list.begin()));
		m_list.pop_front();
		return p;
	}
	return NULL;
}
void	MessageMQ::InMsgMQ(Message *pmsg)
{
	m_list.push_back(pmsg);
}