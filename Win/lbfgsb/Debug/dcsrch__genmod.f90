        !COMPILER-GENERATED INTERFACE MODULE: Sun Apr 29 10:47:50 2012
        MODULE DCSRCH__genmod
          INTERFACE 
            SUBROUTINE DCSRCH(F,G,STP,FTOL,GTOL,XTOL,STPMIN,STPMAX,TASK,&
     &ISAVE,DSAVE)
              REAL(KIND=8) :: F
              REAL(KIND=8) :: G
              REAL(KIND=8) :: STP
              REAL(KIND=8) :: FTOL
              REAL(KIND=8) :: GTOL
              REAL(KIND=8) :: XTOL
              REAL(KIND=8) :: STPMIN
              REAL(KIND=8) :: STPMAX
              CHARACTER(*) :: TASK
              INTEGER(KIND=4) :: ISAVE(2)
              REAL(KIND=8) :: DSAVE(13)
            END SUBROUTINE DCSRCH
          END INTERFACE 
        END MODULE DCSRCH__genmod
