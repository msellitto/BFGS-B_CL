        !COMPILER-GENERATED INTERFACE MODULE: Sun Apr 29 10:47:50 2012
        MODULE CMPRLB__genmod
          INTERFACE 
            SUBROUTINE CMPRLB(N,M,X,G,WS,WY,SY,WT,Z,R,WA,INDEX,THETA,COL&
     &,HEAD,NFREE,CNSTND,INFO)
              INTEGER(KIND=4) :: M
              INTEGER(KIND=4) :: N
              REAL(KIND=8) :: X(N)
              REAL(KIND=8) :: G(N)
              REAL(KIND=8) :: WS(N,M)
              REAL(KIND=8) :: WY(N,M)
              REAL(KIND=8) :: SY(M,M)
              REAL(KIND=8) :: WT(M,M)
              REAL(KIND=8) :: Z(N)
              REAL(KIND=8) :: R(N)
              REAL(KIND=8) :: WA(4*M)
              INTEGER(KIND=4) :: INDEX(N)
              REAL(KIND=8) :: THETA
              INTEGER(KIND=4) :: COL
              INTEGER(KIND=4) :: HEAD
              INTEGER(KIND=4) :: NFREE
              LOGICAL(KIND=4) :: CNSTND
              INTEGER(KIND=4) :: INFO
            END SUBROUTINE CMPRLB
          END INTERFACE 
        END MODULE CMPRLB__genmod
