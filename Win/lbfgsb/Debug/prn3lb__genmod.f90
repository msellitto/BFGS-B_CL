        !COMPILER-GENERATED INTERFACE MODULE: Sun Apr 29 10:47:50 2012
        MODULE PRN3LB__genmod
          INTERFACE 
            SUBROUTINE PRN3LB(N,X,F,TASK,IPRINT,INFO,ITFILE,ITER,NFGV,  &
     &NINTOL,NSKIP,NACT,SBGNRM,TIME,NINT,WORD,IBACK,STP,XSTEP,K,CACHYT, &
     &SBTIME,LNSCHT)
              INTEGER(KIND=4) :: N
              REAL(KIND=8) :: X(N)
              REAL(KIND=8) :: F
              CHARACTER(LEN=60) :: TASK
              INTEGER(KIND=4) :: IPRINT
              INTEGER(KIND=4) :: INFO
              INTEGER(KIND=4) :: ITFILE
              INTEGER(KIND=4) :: ITER
              INTEGER(KIND=4) :: NFGV
              INTEGER(KIND=4) :: NINTOL
              INTEGER(KIND=4) :: NSKIP
              INTEGER(KIND=4) :: NACT
              REAL(KIND=8) :: SBGNRM
              REAL(KIND=8) :: TIME
              INTEGER(KIND=4) :: NINT
              CHARACTER(LEN=3) :: WORD
              INTEGER(KIND=4) :: IBACK
              REAL(KIND=8) :: STP
              REAL(KIND=8) :: XSTEP
              INTEGER(KIND=4) :: K
              REAL(KIND=8) :: CACHYT
              REAL(KIND=8) :: SBTIME
              REAL(KIND=8) :: LNSCHT
            END SUBROUTINE PRN3LB
          END INTERFACE 
        END MODULE PRN3LB__genmod
