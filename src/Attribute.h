#ifndef ATTRIBUTE_H
#define ATTRIBUTE_H

#include <string>
#include <vector>
using namespace std;

class AttributeGenerator;

//Base class with pure virtual methods which will be derived as per different types of attribute generation
class Attribute
{
public:
	//Name of attribute
	string name;
	//Attribute generation probability
	int prob;
	Attribute(string, int);
	//Checks attribute probability and generate is accordingly
	virtual string make_random() = 0;
};

//Generates Boolean attributes such as hot, cold, used, unused, deprecated, etc
class BooleanAttribute : public Attribute
{
public:
	BooleanAttribute(string, int);
	string make_random();
};

//Generates Multi Choice attributes such as visibility("option"), no_sanitize("option"), etc
class MultiChoiceAttribute : public Attribute
{
public:
	//stores various options of attributes e.g. visibility options - default, hidden, internal and protected
	vector<string> choices;
	MultiChoiceAttribute(string, int, vector<string>);
	string make_random();
};

class AttributeGenerator
{
public:
	//stores instances of Attribute
	vector<Attribute*> attributes;
	void Output(std::ostream &);
};

#endif
