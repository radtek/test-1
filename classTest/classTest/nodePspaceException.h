#ifndef _NODEPSPACEEXCEPTION_
#define _NODEPSPACEEXCEPTION_

class NodePspaceException{
private:
	std::string m_message;
public:
	NodePspaceException(std::string message):m_message(message){}
	std::string getMessage(){ return m_message;}
};


#endif