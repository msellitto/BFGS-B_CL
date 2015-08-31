        !COMPILER-GENERATED INTERFACE MODULE: Sun Apr 29 10:47:50 2012
        MODULE MATUPD__genmod
          INTERFACE 
            SUBROUTINE MATUPD(N,M,WS,WY,SY,SS,D,R,ITAIL,IUPDAT,COL,HEAD,&
     &THETA,RR,DR,STP,DTD)
              INTEGER(KIND=4) :: M
              INTEGER(KIND=4) :: N
              REAL(KIND=8) :: WS(N,M)
              REAL(KIND=8) :: WY(N,M)
              REAL(KIND=8) :: SY(M,M)
              REAL(KIND=8) :: SS(M,M)
              REAL(KIND=8) :: D(N)
              REAL(KIND=8) :: R(N)
              INTEGER(KIND=4) :: ITAIL
              INTEGER(KIND=4) :: IUPDAT
              INTEGER(KIND=4) :: COL
              INTEGER(KIND=4) :: HEAD
              REAL(KIND=8) :: THETA
              REAL(KIND=8) :: RR
              REAL(KIND=8) :: DR
              REAL(KIND=8) :: STP
              REAL(KIND=8) :: DTD
            END SUBROUTINE MATUPD
          END INTERFACE 
        END MODULE MATUPD__genmod
