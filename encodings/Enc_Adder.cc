#include "Enc_Adder.h"
#include <algorithm>
#include <numeric>

using namespace openwbo;

void Adder::FA_extra ( Solver *S, Lit xc, Lit xs, Lit a, Lit b, Lit c )
{
  
  clause.clear();
  addTernaryClause(S, ~xc, ~xs, a);
  addTernaryClause(S, ~xc, ~xs, b);
  addTernaryClause(S, ~xc, ~xs, c);

  addTernaryClause(S, xc, xs, ~a);
  addTernaryClause(S, xc, xs, ~b);
  addTernaryClause(S, xc, xs, ~c);
}


Lit Adder::FA_carry ( Solver *S, Lit a, Lit b, Lit c ) {
  
  Lit x = mkLit(S->newVar(), false);

  addTernaryClause(S, b, c, ~x);
  addTernaryClause(S, a, c, ~x);
  addTernaryClause(S, a, b, ~x);

  addTernaryClause(S, ~b, ~c, x);
  addTernaryClause(S, ~a, ~c, x);
  addTernaryClause(S, ~a, ~b, x);

  return x;
}

Lit Adder::FA_sum ( Solver *S, Lit a, Lit b, Lit c )
{
    Lit x = mkLit(S->newVar(), false);

    addQuaternaryClause(S, a, b, c, ~x);
    addQuaternaryClause(S, a, ~b, ~c, ~x);
    addQuaternaryClause(S, ~a, b, ~c, ~x);
    addQuaternaryClause(S, ~a, ~b, c, ~x);

    addQuaternaryClause(S, ~a, ~b, ~c, x);
    addQuaternaryClause(S, ~a, b, c, x);
    addQuaternaryClause(S, a, ~b, c, x);
    addQuaternaryClause(S, a, b, ~c, x);

    return x;
}

Lit Adder::HA_carry ( Solver *S, Lit a, Lit b) // a AND b
{  
  Lit x = mkLit(S->newVar(), false);

  addBinaryClause(S, a, ~x);
  addBinaryClause(S, b, ~x);
  addTernaryClause(S, ~a, ~b, x);

  return x;
}

Lit Adder::HA_sum ( Solver *S, Lit a, Lit b ) // a XOR b
{
  Lit x = mkLit(S->newVar(), false);

  addTernaryClause(S, ~a, ~b, ~x);
  addTernaryClause(S, a, b, ~x);
  
  addTernaryClause(S, ~a, b, x);
  addTernaryClause(S, a, ~b, x);

  return x;
}


void Adder::adderTree (Solver *S, std::vector< std::queue< Lit > > & buckets, vec< Lit >& result ) {
  Lit x,y,z;
  Lit u = lit_Undef;

  for ( int i = 0; i < buckets.size(); i++ ) {
      if ( buckets[i].size() == 0 )
    continue;

      if ( i == buckets.size() - 1 && buckets[i].size() >= 2 ) {
    buckets.push_back ( std::queue<Lit>() );
    result.push ( u );
    }

      while ( buckets[i].size() >= 3 ) {
    x = buckets[i].front();
    buckets[i].pop();
    y = buckets[i].front();
    buckets[i].pop();
    z = buckets[i].front();
    buckets[i].pop();
    Lit xs = FA_sum ( S, x,y,z );
    Lit xc = FA_carry ( S, x,y,z );
    buckets[i  ].push ( xs );
    buckets[i+1].push ( xc );
    FA_extra(S, xc, xs, x, y, z);
    }

      if ( buckets[i].size() == 2 ) {
    x = buckets[i].front();
    buckets[i].pop();
    y = buckets[i].front();
    buckets[i].pop();
    buckets[i  ].push ( HA_sum ( S, x,y ) );
    buckets[i+1].push ( HA_carry ( S, x,y ) );
    }


      result[i] = buckets[i].front();
      buckets[i].pop();
      }

  }

  // Generates clauses for “xs <= ys”, assuming ys has only constant signals (0 or 1).
// xs and ys must have the same size

void Adder::lessThanOrEqual (Solver *S, vec< Lit > & xs, std::vector< uint64_t > & ys) {
  assert ( xs.size() == ys.size() );
  vec<Lit> clause;
  bool skip;
  for ( int i = 0; i < xs.size(); ++i ) {
      if ( ys[i] == 1 || xs[i] == lit_Undef )
    continue;
      
      clause.clear();

      skip = false;

      for ( int j = i + 1; j < xs.size(); ++j )
      {
    if ( ys[j] == 1 )
    {
        if ( xs[j] == lit_Undef )
        {
      skip = true;
      break;
        }

        clause.push ( ~xs[j] );
    }
    else
    {
        assert ( ys[j] == 0 );

        if ( xs[j] == lit_Undef )
      continue;

        clause.push ( xs[j] );
    }
      }

      if ( skip )
    continue;

      clause.push ( ~xs[i] );

      //formula.addClause( clause );
      S->addClause(clause);
      }

}

void Adder::lessThanOrEqualInc (Solver *S, vec< Lit > & xs, std::vector< uint64_t > & ys, vec<Lit>& assumptions) {
  assert ( xs.size() == ys.size() );
  vec<Lit> clause;
  bool skip;
  for ( int i = 0; i < xs.size(); ++i ) {
      if ( ys[i] == 1 || xs[i] == lit_Undef )
    continue;
      
      clause.clear();

      skip = false;

      for ( int j = i + 1; j < xs.size(); ++j )
      {
    if ( ys[j] == 1 )
    {
        if ( xs[j] == lit_Undef )
        {
      skip = true;
      break;
        }

        clause.push ( ~xs[j] );
    }
    else
    {
        assert ( ys[j] == 0 );

        if ( xs[j] == lit_Undef )
      continue;

        clause.push ( xs[j] );
    }
      }

      if ( skip )
    continue;

      clause.push ( ~xs[i] );

      //formula.addClause( clause );
      Lit t = mkLit(S->newVar(), false);
      clause.push(t);
      assumptions.push(~t);
      S->addClause(clause);
      }

}

void Adder::numToBits ( std::vector<uint64_t> & bits, uint64_t n, uint64_t number ) {
    bits.clear();

  
  for ( int64_t i = n - 1; i >= 0; --i ) {
      int64_t tmp = ((int64_t)1) << i;
      if ( number < tmp ) {
    bits.push_back ( 0 );
    }
      else {
    bits.push_back ( 1 );
    number -= tmp;
    }
      }

  reverse ( bits.begin(), bits.end() );
}

void Adder::encode(Solver *S, vec<Lit> &lits, vec<uint64_t> &coeffs, uint64_t rhs){

    _output.clear();

    uint64_t nb = ld64(rhs); // number of bits
    Lit u = lit_Undef;

    for ( int iBit = 0; iBit < nb; ++iBit ) {
        _buckets.push_back ( std::queue<Lit>() );
        _output.push ( u );
        for ( int iVar = 0; iVar < lits.size(); ++iVar ) {
            if ( ( ( ((int64_t)1) << iBit ) & coeffs[iVar] ) != 0 )
                _buckets.back().push ( lits[iVar] );
            }
        }

    std::vector<uint64_t> kBits;

    adderTree (S, _buckets, _output);
  
    numToBits (kBits, _buckets.size(), rhs );

    lessThanOrEqual (S, _output, kBits);
    hasEncoding = true;
}

void Adder::encodeInc(Solver *S, vec<Lit> &lits, vec<uint64_t> &coeffs, uint64_t rhs, vec<Lit> &assumptions){
    _output.clear();

    uint64_t nb = ld64(rhs); // number of bits
    Lit u = lit_Undef;

    for ( int iBit = 0; iBit < nb; ++iBit ) {
        _buckets.push_back ( std::queue<Lit>() );
        _output.push ( u );
        for ( int iVar = 0; iVar < lits.size(); ++iVar ) {
            if ( ( ( ((int64_t)1) << iBit ) & coeffs[iVar] ) != 0 )
                _buckets.back().push ( lits[iVar] );
            }
        }

    std::vector<uint64_t> kBits;

    adderTree (S, _buckets, _output);
    numToBits (kBits, _buckets.size(), rhs );

    lessThanOrEqualInc (S, _output, kBits, assumptions);
    hasEncoding = true;

}

void Adder::updateInc(Solver *S, uint64_t rhs, vec<Lit>& assumptions){
      
      std::vector<uint64_t> kBits;
      numToBits (kBits, _buckets.size(), rhs );
      lessThanOrEqualInc (S, _output, kBits, assumptions);
}

void Adder::update(Solver *S, uint64_t rhs){
      
      std::vector<uint64_t> kBits;
      numToBits (kBits, _buckets.size(), rhs );
      lessThanOrEqual (S, _output, kBits);
}


uint64_t Adder::ld64(const uint64_t x)
{
  return (sizeof(uint64_t) << 3) - __builtin_clzll (x);
//   cout << "x " << x << endl;
//   int ldretutn = 0;
//   for (int i = 0; i < 63; ++i)
//   {
//     if ((x & (1 << i)) > 0)
//     {
//       cout << "ldretutn " << ldretutn << endl;
//       ldretutn = i + 1;
//     }
//   }
//   
//   return ldretutn;
}












