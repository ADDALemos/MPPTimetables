using NSPACE::vec;

// A singleton which manages TorcOpenWbo's additional (w.r.t OpenWboInc) parameters and functionality
class Torc{
public:
   static Torc* Instance();
   
   void SetPolConservative(bool isConservative) { polIsConservative = isConservative; }
   void SetConservativeAllVars(bool isConservativeAllVars) { conservativeUseAllVars = isConservativeAllVars; }
   void SetPolOptimistic(bool isOptimistic) { polIsOptimistic = isOptimistic;}
   void SetTargetVarsBumpVal(int targetVarsBumpVal) { varTargetsBumpVal = targetVarsBumpVal; }   
   void SetBumpRelWeights(bool isBumpRelWeights) { bumpRelWeights = isBumpRelWeights; }   
   void SetTargetBumpMaxRandVal(int targetVarsBumpRandVal) { varTargetsBumpMaxRandVal = targetVarsBumpRandVal; }   
   
   bool GetPolConservative() const { return polIsConservative; }
   bool GetConservativeAllVars() const { return conservativeUseAllVars; } 
   bool GetPolOptimistic() const { return polIsOptimistic; }
   int GetTargetVarsBumpVal() const { return varTargetsBumpVal; }   
   bool GetBumpRelWeights() const { return bumpRelWeights; }
   int GetTargetBumpMaxRandVal() const { return varTargetsBumpMaxRandVal; }   
   
   int GetRandBump() const;
   
   vec<bool>& TargetIsVarTarget() { return isVarTarget; }
private:
   Torc() : polIsConservative(true), conservativeUseAllVars(true), polIsOptimistic(true), varTargetsBumpVal(113), bumpRelWeights(false), varTargetsBumpMaxRandVal(0)  {};  // Private so that it can  not be called
   static Torc* m_pInstance;
   
   bool polIsConservative;
   bool conservativeUseAllVars;
   bool polIsOptimistic;
   int varTargetsBumpVal;  
   bool bumpRelWeights;    
   int varTargetsBumpMaxRandVal;  
   
   vec<bool> isVarTarget;
};


