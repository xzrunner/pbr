#pragma once

#include <string>
#include <vector>
#include <memory>

namespace pbr
{

class CodeNode
{
public:
	virtual ~CodeNode() {}

	virtual bool ExistLine(const std::string& line) const = 0;

	virtual void ToText(std::string& text) const = 0;

}; // CodeNode

class CodeLine : public CodeNode
{
public:
	CodeLine(const std::string& line) : m_str(line) {}

	virtual bool ExistLine(const std::string& line) const override {
		return m_str == line;
	}

	virtual void ToText(std::string& text) const override {
		text += m_str + "\n";
	}

private:
	std::string m_str;

}; // CodeLine

class CodeBlock : public CodeNode
{
public:
	virtual bool ExistLine(const std::string& line) const override;

	virtual void ToText(std::string& text) const override;

	void Add(const std::shared_ptr<CodeNode>& node) {
		m_children.push_back(node);
	}

	void SetHeader(const std::string& header) {
		m_header = header;
	}

private:
	std::vector<std::shared_ptr<CodeNode>> m_children;

	std::string m_header;

}; // CodeBlock

class CodeGenerator
{
public:
    CodeGenerator();

	void Line(const std::string& s = "");
    void LineFmt(const std::string str, ...);
	void Block(CodeGenerator& gen);

	bool ExistLine(const std::string& line) const;

	void Tab();
	void DeTab();

	std::string ToText() const;

private:
	std::shared_ptr<CodeBlock> m_block = nullptr;

	std::string m_header;

}; // CodeGenerator

}