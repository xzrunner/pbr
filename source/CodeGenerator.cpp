#include "pbr/CodeGenerator.h"

#include <stdarg.h>

namespace pbr
{

//////////////////////////////////////////////////////////////////////////
// class CodeBlock
//////////////////////////////////////////////////////////////////////////

bool CodeBlock::ExistLine(const std::string& line) const
{
	for (auto& c : m_children) {
        if (c->ExistLine(line)) {
            return true;
        }
	}
	return false;
}

void CodeBlock::ToText(std::string& text) const
{
	if (m_header.empty())
	{
        for (auto& c : m_children) {
            c->ToText(text);
        }
	}
	else
	{
        for (auto& c : m_children)
		{
			text += m_header;
			c->ToText(text);
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// class CodeGenerator
//////////////////////////////////////////////////////////////////////////

CodeGenerator::CodeGenerator()
{
    m_block = std::make_shared<CodeBlock>();
}

void CodeGenerator::Line(const std::string& s/* = ""*/)
{
	m_block->Add(std::make_shared<CodeLine>(m_header + s));
}

void CodeGenerator::LineFmt(const std::string fmt, ...)
{
	int size = ((int)fmt.size()) * 2 + 50;   // Use a rubric appropriate for your code
	std::string str;
	va_list ap;
	while (1) {     // Maximum two passes on a POSIX system...
		str.resize(size);
		va_start(ap, fmt);
		int n = vsnprintf((char *)str.data(), size, fmt.c_str(), ap);
		va_end(ap);
		if (n > -1 && n < size) {  // Everything worked
			str.resize(n);
			break;
		}
		if (n > -1)  // Needed size returned
			size = n + 1;   // For null char
		else
			size *= 2;      // Guess at a larger size (OS specific)
	}
    Line(str);
}

void CodeGenerator::Block(CodeGenerator& gen)
{
	gen.m_block->SetHeader(m_header);
	m_block->Add(gen.m_block);
}

bool CodeGenerator::ExistLine(const std::string& line) const
{
	return m_block->ExistLine(line);
}

void CodeGenerator::Tab()
{
	m_header += "\t";
}

void CodeGenerator::DeTab()
{
	m_header = m_header.substr(0, m_header.find_last_of("\t"));
}

std::string CodeGenerator::ToText() const
{
	std::string str;
	m_block->ToText(str);
	return str;
}

}