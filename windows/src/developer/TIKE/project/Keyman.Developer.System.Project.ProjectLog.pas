(*
  Name:             Keyman.Developer.System.Project.ProjectLog
  Copyright:        Copyright (C) SIL International.
  Documentation:    
  Description:      
  Create Date:      11 May 2015

  Modified Date:    11 May 2015
  Authors:          mcdurdin
  Related Files:    
  Dependencies:     

  Bugs:             
  Todo:             
  Notes:            
  History:          11 May 2015 - mcdurdin - I4706 - V9.0 - Update compile logging for silent and warning-as-error cleanness
                    
*)
unit Keyman.Developer.System.Project.ProjectLog;   // I4706

interface

type
  TProjectLogState = (plsInfo, plsWarning, plsError, plsFatal);

type
  TCompilePackageMessageEvent = procedure(Sender: TObject; msg: string; State: TProjectLogState) of object;

implementation

end.
