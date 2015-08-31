        !COMPILER-GENERATED INTERFACE MODULE: Sun Apr 29 10:56:58 2012
        MODULE FORMK__genmod
          INTERFACE 
            SUBROUTINE FORMK(N,NSUB,IND,NENTER,ILEAVE,INDX2,IUPDAT,     &
     &UPDATD,WN,WN1,M,WS,WY,SY,THETA,COL,HEAD,INFO)
              INTEGER(KIND=4) :: M
              INTEGER(KIND=4) :: N
              INTEGER(KIND=4) :: NSUB
              INTEGER(KIND=4) :: IND(N)
              INTEGER(KIND=4) :: NENTER
              INTEGER(KIND=4) :: ILEAVE
              INTEGER(KIND=4) :: INDX2(N)
              INTEGER(KIND=4) :: IUPDAT
              LOGICAL(KIND=4) :: UPDATD
              REAL(KIND=8) :: WN(2*M,2*M)
              REAL(KIND=8) :: WN1(2*M,2*M)
              REAL(KIND=8) :: WS(N,M)
              REAL(KIND=8) :: WY(N,M)
              REAL(KIND=8) :: SY(M,M)
              REAL(KIND=8) :: THETA
              INTEGER(KIND=4) :: COL
              INTEGER(KIND=4) :: HEAD
              INTEGER(KIND=4) :: INFO
            END SUBROUTINE FORMK
          END INTERFACE 
        END MODULE FORMK__genmod
