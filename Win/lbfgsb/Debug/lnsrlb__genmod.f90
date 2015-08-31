        !COMPILER-GENERATED INTERFACE MODULE: Sun Apr 29 10:47:50 2012
        MODULE LNSRLB__genmod
          INTERFACE 
            SUBROUTINE LNSRLB(N,L,U,NBD,X,F,FOLD,GD,GDOLD,G,D,R,T,Z,STP,&
     &DNORM,DTD,XSTEP,STPMX,ITER,IFUN,IBACK,NFGV,INFO,TASK,BOXED,CNSTND,&
     &CSAVE,ISAVE,DSAVE)
              INTEGER(KIND=4) :: N
              REAL(KIND=8) :: L(N)
              REAL(KIND=8) :: U(N)
              INTEGER(KIND=4) :: NBD(N)
              REAL(KIND=8) :: X(N)
              REAL(KIND=8) :: F
              REAL(KIND=8) :: FOLD
              REAL(KIND=8) :: GD
              REAL(KIND=8) :: GDOLD
              REAL(KIND=8) :: G(N)
              REAL(KIND=8) :: D(N)
              REAL(KIND=8) :: R(N)
              REAL(KIND=8) :: T(N)
              REAL(KIND=8) :: Z(N)
              REAL(KIND=8) :: STP
              REAL(KIND=8) :: DNORM
              REAL(KIND=8) :: DTD
              REAL(KIND=8) :: XSTEP
              REAL(KIND=8) :: STPMX
              INTEGER(KIND=4) :: ITER
              INTEGER(KIND=4) :: IFUN
              INTEGER(KIND=4) :: IBACK
              INTEGER(KIND=4) :: NFGV
              INTEGER(KIND=4) :: INFO
              CHARACTER(LEN=60) :: TASK
              LOGICAL(KIND=4) :: BOXED
              LOGICAL(KIND=4) :: CNSTND
              CHARACTER(LEN=60) :: CSAVE
              INTEGER(KIND=4) :: ISAVE(2)
              REAL(KIND=8) :: DSAVE(13)
            END SUBROUTINE LNSRLB
          END INTERFACE 
        END MODULE LNSRLB__genmod
