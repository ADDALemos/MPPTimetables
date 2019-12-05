# How to compile the project

`make`

# How to run the project

`./timetabler data/input/ITC-2019/OriginalProblem.xml -formula=1 -verbosity=0 -algorithm=6 -pb=0 -opt-allocation -opt-stu -opt-cons [options]`

This will generate a valid solution in the output folder. The results is encoded using [ITC-2019 XML format](https://www.itc2019.org/home).
Note that, to solve Minimal Perturbation Problem one needs to have a solution for the original problem in the folder  `data/output/ITC-2019/solution-OriginalProblem.xml`.


This program is based on [TT-Open-WBO-INC]. The following options are available:

## Timetabler Options
### Optimization Criterion: allocation
```-opt-allocation      (default: off)```
### Optimization Criterion: distribution constraints
```-opt-cons      (default: off)```
### Optimization Criterion: student conflicts
```-opt-stu      (default: off)```
### Minimal Perturbation Problem: invalid time flag
```invalid-time      (default: on)```
### Minimal Perturbation Problem: invalid time probability
```invalid-time-p = <int32>  [   0 ..    100]     (default: 21%)```
### Minimal Perturbation Problem: invalid room flag
```invalid-room    (default: on)```
### Minimal Perturbation Problem: invalid room probability
```invalid-room-p = <int32>  [   0 ..    100]     (default: 25%)```

## Global Options
### Formula type (0=MaxSAT, 1=PB)
```-formula      = <int32>  [   0 ..    1] (default: 0)```

### Print model
```-print-model, -no-print-model (default on)```

### Write unsatisfied soft clauses to file
```-print-unsat-soft = <output-file>```

### Verbosity level (0=minimal, 1=more)
```-verbosity    = <int32>  [   0 ..    1] (default: 1)```

### Search algorithm (0=wbo,1=linear-su,2=msu3,3=part-msu3,4=oll,5=best)
```-algorithm    = <int32>  [   0 ..    1] (default: 5)```

### BMO search 
```-bmo,-no-bmo (default on)```

### Pseudo-Boolean encodings (0=SWC,1=GTE, 2=Adder)
```-pb           = <int32>  [   0 ..    1] (default: 1)```

### At-most-one encodings (0=ladder)
```-amo          = <int32>  [   0 ..    0] (default: 0)```

### Cardinality encodings (0=cardinality networks, 1=totalizer, 2=modulo totalizer)
```-cardinality  = <int32>  [   0 ..    2] (default: 1)```

       
## WBO Options (algorithm=0, unsatisfiability-based algorithm)
### Weight strategy (0=none, 1=weight-based, 2=diversity-based)
```-weight-strategy = <int32>  [   0 ..    2] (default: 2)```

### Symmetry breaking
```-symmetry, -no-symmetry (default on)```

### Limit on the number of symmetry breaking clauses
```-symmetry-limit = <int32>  [   0 .. imax] (default: 500000)```

## PartMSU3 OPTIONS (algorithm=3, partition-based algorithm)
### Graph type (0=vig, 1=cvig, 2=res)
```-graph-type   = <int32>  [   0 ..    2] (default: 2)```

### Partition strategy (0=sequential, 1=sequential-sorted, 2=binary)
```-partition-strategy = <int32>  [   0 ..    2] (default: 2)```

    
# Dependencies
   
   [TT-Open-WBO-INC](https://drive.google.com/file/d/140d8jDHZHo5d7WuoNpLqZXmHasgYkH38/view) solver and c++ compiler.
   
  # Data Sets
   
   ## IST
   
   The folders `data/input/IST` and `data/output/IST` contain the data sets from Instituto Superior Técnico (IST). 
   
   ## ITC-2007
   
   The folders `data/input/ITC-2007` and `data/output/ITC-2007` contain the data sets from ITC-2007 Curriculum based timetabling track. The results were obtained from [Lindahl et al.](http://github.com/miclindahl/UniTimetabling).
   
   ## ITC-2019

   The folder `data/input/ITC-2019` contains some data sets from ITC-2019.
   
   ## Disruptions Profiles
   
   The folder `data/Disruption/profiles/` contains disruptions learn from the timetables of the last 5 years of IST.
   
  # Results
  
  This solver has been placed among the five finalists of ITC-2019. The folder `papers/` contains the related papers.
  
  ## Journal of Scheduling and other approaches
  
  For an integer programming approach see Journal of Scheduling release. Note that, integer programming approach is not based on open source software.
