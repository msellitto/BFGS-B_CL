        !COMPILER-GENERATED INTERFACE MODULE: Sun Apr 29 10:56:58 2012
        MODULE CAUCHY__genmod
          INTERFACE 
            SUBROUTINE CAUCHY(N,X,L,U,NBD,G,IORDER,IWHERE,T,D,XCP,M,WY, &
     &WS,SY,WT,THETA,COL,HEAD,P,C,WBP,V,NINT,IPRINT,SBGNRM,INFO,EPSMCH)
              INTEGER(KIND=4) :: COL
              INTEGER(KIND=4) :: M
              INTEGER(KIND=4) :: N
              REAL(KIND=8) :: X(N)
              REAL(KIND=8) :: L(N)
              REAL(KIND=8) :: U(N)
              INTEGER(KIND=4) :: NBD(N)
              REAL(KIND=8) :: G(N)
              INTEGER(KIND=4) :: IORDER(N)
              INTEGER(KIND=4) :: IWHERE(N)
              REAL(KIND=8) :: T(N)
              REAL(KIND=8) :: D(N)
              REAL(KIND=8) :: XCP(N)
              REAL(KIND=8) :: WY(N,COL)
              REAL(KIND=8) :: WS(N,COL)
              REAL(KIND=8) :: SY(M,M)
              REAL(KIND=8) :: WT(M,M)
              REAL(KIND=8) :: THETA
              INTEGER(KIND=4) :: HEAD
              REAL(KIND=8) :: P(2*M)
              REAL(KIND=8) :: C(2*M)
              REAL(KIND=8) :: WBP(2*M)
              REAL(KIND=8) :: V(2*M)
              INTEGER(KIND=4) :: NINT
              INTEGER(KIND=4) :: IPRINT
              REAL(KIND=8) :: SBGNRM
              INTEGER(KIND=4) :: INFO
              REAL(KIND=8) :: EPSMCH
            END SUBROUTINE CAUCHY
          END INTERFACE 
        END MODULE CAUCHY__genmod
