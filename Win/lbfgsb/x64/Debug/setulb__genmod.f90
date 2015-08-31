        !COMPILER-GENERATED INTERFACE MODULE: Sun Apr 29 10:56:58 2012
        MODULE SETULB__genmod
          INTERFACE 
            SUBROUTINE SETULB(N,M,X,L,U,NBD,F,G,FACTR,PGTOL,WA,IWA,TASK,&
     &IPRINT,CSAVE,LSAVE,ISAVE,DSAVE)
              INTEGER(KIND=4) :: M
              INTEGER(KIND=4) :: N
              REAL(KIND=8) :: X(N)
              REAL(KIND=8) :: L(N)
              REAL(KIND=8) :: U(N)
              INTEGER(KIND=4) :: NBD(N)
              REAL(KIND=8) :: F
              REAL(KIND=8) :: G(N)
              REAL(KIND=8) :: FACTR
              REAL(KIND=8) :: PGTOL
              REAL(KIND=8) :: WA(2*M*N+4*N+11*M*M+8*M)
              INTEGER(KIND=4) :: IWA(3*N)
              CHARACTER(LEN=60) :: TASK
              INTEGER(KIND=4) :: IPRINT
              CHARACTER(LEN=60) :: CSAVE
              LOGICAL(KIND=4) :: LSAVE(4)
              INTEGER(KIND=4) :: ISAVE(44)
              REAL(KIND=8) :: DSAVE(29)
            END SUBROUTINE SETULB
          END INTERFACE 
        END MODULE SETULB__genmod
