        !COMPILER-GENERATED INTERFACE MODULE: Sun Apr 29 10:47:50 2012
        MODULE PRN2LB__genmod
          INTERFACE 
            SUBROUTINE PRN2LB(N,X,F,G,IPRINT,ITFILE,ITER,NFGV,NACT,     &
     &SBGNRM,NINT,WORD,IWORD,IBACK,STP,XSTEP)
              INTEGER(KIND=4) :: N
              REAL(KIND=8) :: X(N)
              REAL(KIND=8) :: F
              REAL(KIND=8) :: G(N)
              INTEGER(KIND=4) :: IPRINT
              INTEGER(KIND=4) :: ITFILE
              INTEGER(KIND=4) :: ITER
              INTEGER(KIND=4) :: NFGV
              INTEGER(KIND=4) :: NACT
              REAL(KIND=8) :: SBGNRM
              INTEGER(KIND=4) :: NINT
              CHARACTER(LEN=3) :: WORD
              INTEGER(KIND=4) :: IWORD
              INTEGER(KIND=4) :: IBACK
              REAL(KIND=8) :: STP
              REAL(KIND=8) :: XSTEP
            END SUBROUTINE PRN2LB
          END INTERFACE 
        END MODULE PRN2LB__genmod
