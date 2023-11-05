#if HAVE_CONFIG_H
#  include <config.h>
#endif

#ifdef WIN32
#pragma warning(disable : 4786)   /* Disable annoying warning messages */
#endif

#include "random.h"
#include "OutputMgr.h"
#include "Attribute.h"

void
AttributeGenerator::Output(std::ostream &out)
{
	bool attr_emitted = false;
	vector<Attribute*>::iterator itr;
	for(itr = attributes.begin(); itr != attributes.end(); itr++){
		string attr_generated;
		attr_generated = (*itr)->make_random();
		if(attr_generated != ""){
			if(!attr_emitted){
				out << " __attribute__((" << attr_generated;
				attr_emitted = true;
			}
			else
				out << ", " << attr_generated;
		}
	}
	if(attr_emitted)
		out << "))";
}

Attribute::Attribute(const string &name, int prob)
	:  name(name), prob(prob)
{
}

BooleanAttribute::BooleanAttribute(const string &name, int prob)
	: Attribute(name, prob)
{
}

string
BooleanAttribute::make_random()
{
	if(rnd_flipcoin(prob))
		return name;
	else
		return "";
}

MultiChoiceAttribute::MultiChoiceAttribute(const string &name, int prob, vector<string> arguments)
	: Attribute(name, prob), choices(arguments)
{
}

string
MultiChoiceAttribute::make_random()
{
	if(rnd_flipcoin(prob))
		return name + "(\"" + choices[rnd_upto(choices.size())] + "\")";
	else
		return "";
}

AlignedAttribute::AlignedAttribute(const string &name, int prob, int alignment_factor)
	: Attribute(name, prob), alignment(alignment_factor)
{
}

string
AlignedAttribute::make_random()
{
	if(rnd_flipcoin(prob)){
		ostringstream oss;
		oss << (1 << rnd_upto(alignment));
		return name + "(" + oss.str() + ")";
	}
	else
		return "";
}

SectionAttribute::SectionAttribute(const string &name, int prob)
	: Attribute(name, prob)
{
}

string
SectionAttribute::make_random()
{
	if(rnd_flipcoin(prob)){
		ostringstream oss;
		oss << rnd_upto(10);
		return name + "(\"usersection" + oss.str() + "\")";
	}
	else
		return "";
}
