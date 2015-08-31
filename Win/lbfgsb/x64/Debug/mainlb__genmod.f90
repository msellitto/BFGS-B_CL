        !COMPILER-GENERATED INTERFACE MODULE: Sun Apr 29 10:56:58 2012
        MODULE MAINLB__genmod
          INTERFACE 
            SUBROUTINE MAINLB(N,M,X,L,U,NBD,F,G,FACTR,PGTOL,WS,WY,SY,SS,&
     &WT,WN,SND,Z,R,D,T,WA,INDEX,IWHERE,INDX2,TASK,IPRINT,CSAVE,LSAVE,  &
     &ISAVE,DSAVE)
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
              REAL(KIND=8) :: WS(N,M)
              REAL(KIND=8) :: WY(N,M)
              REAL(KIND=8) :: SY(M,M)
              REAL(KIND=8) :: SS(M,M)
              REAL(KIND=8) :: WT(M,M)
              REAL(KIND=8) :: WN(2*M,2*M)
              REAL(KIND=8) :: SND(2*M,2*M)
              REAL(KIND=8) :: Z(N)
              REAL(KIND=8) :: R(N)
              REAL(KIND=8) :: D(N)
              REAL(KIND=8) :: T(N)
              REAL(KIND=8) :: WA(8*M)
              INTEGER(KIND=4) :: INDEX(N)
              INTEGER(KIND=4) :: IWHERE(N)
              INTEGER(KIND=4) :: INDX2(N)
              CHARACTER(LEN=60) :: TASK
              INTEGER(KIND=4) :: IPRINT
              CHARACTER(LEN=60) :: CSAVE
              LOGICAL(KIND=4) :: LSAVE(4)
              INTEGER(KIND=4) :: ISAVE(23)
              REAL(KIND=8) :: DSAVE(29)
            END SUBROUTINE MAINLB
          END INTERFACE 
        END MODULE MAINLB__genmod
