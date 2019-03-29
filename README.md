# How to run the project

`./timetabler OriginalProblem.xml OriginalSolution.xml DisruptionsProfile.p Distance_Metric[Hamming|Weighted|GAP] Model[Boolean|Integer|Mixed|GRASP|LNS] [model_parameters] [-c] [-w]`

This will generate a valid solution in the output folder. The results is encoded using [ITC-2019 XML format](https://www.itc2019.org/home).

# Arguments of the program

## OriginalProblem

The data instance of the problem was to be encoded using [ITC-2019 XML format](https://www.itc2019.org/home). It is possible to use the XML encoding of [ITC-2007](http://www.cs.qub.ac.uk/itc2007/) as well.

## OriginalProblem

The solution for the data instance was to be encoded using [ITC-2019 XML format](https://www.itc2019.org/home). It is possible to use the XML encoding of [ITC-2007](http://www.cs.qub.ac.uk/itc2007/) as well.

## Disruptions Profiles

The file with the rules to randomly generate the disruptions.
 
### Format
The format is separated into four types of disruptions due to their common characteristics. 

* `Disruptions_type[Overlap|Preference_Room|Invalid_Room|Preference_Time|Invalid_Time|Remove_Room|Invalid_Assignment] %percentage_of_occurrences`
* `Remove_Room_Day %percentage_of_closed_down_rooms %percentage_of_days` 
* `Disruptions_type[Modify_Enrolments|Modify_N_Lectures] %percentage_of_occurrences distribution_mean distribution_standard_deviation`
* `Disruptions_type #Lectures Average_Length Average_Enrollments`

## Distance_Metric

There are three distance metrics available:
 - Hamming Distance
 - Weighted Hamming Distance (the number of students affected by the change)
 - GAP: reducing the number of GAPs in students timetable.
 
## Model

There are three LP models that require Gurobi and two local search methods.

The three LP models are:
  - Boolean Model - two Boolean variables
  - Integer Model - one integer variable
  - Mixed Model - one integer and one Boolean variable
  
 The two local search methods:
  - GRASP
  - LNS
  
  ## Model Parameters
  
  The **Mixed** model requires an additional parameter to specify the type of approach. The problem can be solved by:
  - 1: splitting into two problems
  - 0: solve the complete problem
  
  **GRASP** method require two additional parameters;
   - The number of iterations
   - The size of the Restricted Candidate List
   
   ## Symmetry Cuts
   
   To activate symmetry cuts run with the flag -c.
   
   ## Warm Start
   
   To activate the warm start procedure run with the flag -w.
    
   # Dependencies
   
   [Gurobi](http://www.gurobi.com/index) solver and c++ compiler.
   
    
   # Data Sets
   
   ## IST
   
   The folders `data/input/IST` and `data/output/IST` contain the data sets from Instituto Superior Técnico (IST). 
   
   ## ITC-2007
   
   The folders `data/input/ITC-2007` and `data/output/ITC-2007` contain the data sets from ITC-2007 Curriculum based timetabling track. The results were obtained from [Lindahl et al.](http://github.com/miclindahl/UniTimetabling).
   
   ## ITC-2019

   The folder `data/input/ITC-2019` contains some data sets from ITC-2019.
   
   ## Disruptions Profiles
   
   The folder `data/Disruption/profiles/` contains disruptions learn from the timetables of the last 5 years of IST.
   
   # Authors
   
   [Alexandre Lemos](http://web.ist.utl.pt/ist173316/), [Pedro T. Monteiro](http://pedromonteiro.org/) and [Inês Lynce](http://sat.inesc-id.pt/~ines/).
   # Contacts:
   
   If you have any comments or questions, please [contact us](mailto:ines.lynce@tecnico.ulisboa.pt;alexandre.lemos@tecnico.ulisboa.pt;pedro.tiago.monteiro@tecnico.pt;?subject=[Disruption_Timetables]).
