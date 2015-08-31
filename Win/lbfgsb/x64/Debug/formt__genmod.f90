        !COMPILER-GENERATED INTERFACE MODULE: Sun Apr 29 10:56:58 2012
        MODULE FORMT__genmod
          INTERFACE 
            SUBROUTINE FORMT(M,WT,SY,SS,COL,THETA,INFO)
              INTEGER(KIND=4) :: M
              REAL(KIND=8) :: WT(M,M)
              REAL(KIND=8) :: SY(M,M)
              REAL(KIND=8) :: SS(M,M)
              INTEGER(KIND=4) :: COL
              REAL(KIND=8) :: THETA
              INTEGER(KIND=4) :: INFO
            END SUBROUTINE FORMT
          END INTERFACE 
        END MODULE FORMT__genmod
