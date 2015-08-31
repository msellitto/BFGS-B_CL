        !COMPILER-GENERATED INTERFACE MODULE: Sun Apr 29 10:56:58 2012
        MODULE SUBSM__genmod
          INTERFACE 
            SUBROUTINE SUBSM(N,M,NSUB,IND,L,U,NBD,X,D,WS,WY,THETA,COL,  &
     &HEAD,IWORD,WV,WN,IPRINT,INFO)
              INTEGER(KIND=4) :: NSUB
              INTEGER(KIND=4) :: M
              INTEGER(KIND=4) :: N
              INTEGER(KIND=4) :: IND(NSUB)
              REAL(KIND=8) :: L(N)
              REAL(KIND=8) :: U(N)
              INTEGER(KIND=4) :: NBD(N)
              REAL(KIND=8) :: X(N)
              REAL(KIND=8) :: D(N)
              REAL(KIND=8) :: WS(N,M)
              REAL(KIND=8) :: WY(N,M)
              REAL(KIND=8) :: THETA
              INTEGER(KIND=4) :: COL
              INTEGER(KIND=4) :: HEAD
              INTEGER(KIND=4) :: IWORD
              REAL(KIND=8) :: WV(2*M)
              REAL(KIND=8) :: WN(2*M,2*M)
              INTEGER(KIND=4) :: IPRINT
              INTEGER(KIND=4) :: INFO
            END SUBROUTINE SUBSM
          END INTERFACE 
        END MODULE SUBSM__genmod
