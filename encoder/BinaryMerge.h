#ifndef BINARY_MERGE_H
#define BINARY_MERGE_H
#include "../IncSimplePBConstraint.h"
#include "../SimplePBConstraint.h"
#include "../PBConfig.h"
#include "../clausedatabase.h"
#include "../auxvarmanager.h"
#include "../weightedlit.h"
#include <unordered_map>
#include "cardencoding.h"
#include "Encoder.h"

// @inproceedings{enc:binarymerge,
//  author = {Norbert Manthey and Tobias Philipp and Peter Steinke},
//  title = {A More Compact Translation of Pseudo-{Boolean} Constraints into {CNF} such that Generalized Arc Consistency is Maintained},
//  booktitle = {KI 2014: Advances in Artificial Intelligence},
//  series = {Lecture Notes in Computer Science},
//  volume = {8736},
//  editor = {Carsten Lutz and Michael Tielscher},
//  publisher={Springer Berlin Heidelberg},
//  pages={123--134},
//  language={English},
//  isbn={978-3-319-11205-3},
//  year = {2014},
// }

class BinaryMerge : public Encoder
{
private:
  int true_lit;
  CardEncoding old_card_encoder;
  
  void binary_merge(const SimplePBConstraint& constraint, ClauseDatabase& formula, AuxVarManager& auxvars, int32_t gac_lit = 0);
  
  void totalizer(std::vector< int32_t > const & input, std::vector< int32_t > & output, ClauseDatabase& formula, AuxVarManager& auxvars); // needed for the watchdog instantiation
  void unary_adder(std::vector< int32_t > const & a, std::vector< int32_t > const & b, std::vector< int32_t > & c, ClauseDatabase& formula, AuxVarManager& auxvars); // needed for the watchdog instantiation
  
public:
  void encode(const SimplePBConstraint& pbconstraint, ClauseDatabase & formula, AuxVarManager & auxvars);
  int64_t encodingValue(const SimplePBConstraint& pbconstraint);
  
  BinaryMerge(PBConfig & config);
};

#endif // BINARY_MERGE_H
