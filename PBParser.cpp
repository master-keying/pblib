#include "PBParser.h"

using namespace std;

vector<PBLib::PBConstraint> PBParser::parseFile(string fileName)
{
	ok = true;
	SimpleParser<DefaultCallback> parser(fileName.c_str());

	parser.setAutoLinearize(false);
	parser.parse();
	
	ok = parser.isOk();
	
	if (!ok)
	{
	  return vector<PBLib::PBConstraint>();
	}

	// copy constraints from parser
	
	maxVarID = parser.cb.getMaxVarId();
	hasObjective = parser.cb.getIsObtInstance();
	
	if (parser.cb.getIsObtInstance())
		obt_constraint = parser.cb.getObtConstraint();
	
	
	return parser.cb.getConstraints();
}