        !COMPILER-GENERATED INTERFACE MODULE: Sun Apr 29 10:47:50 2012
        MODULE BMV__genmod
          INTERFACE 
            SUBROUTINE BMV(M,SY,WT,COL,V,P,INFO)
              INTEGER(KIND=4) :: COL
              INTEGER(KIND=4) :: M
              REAL(KIND=8) :: SY(M,M)
              REAL(KIND=8) :: WT(M,M)
              REAL(KIND=8) :: V(2*COL)
              REAL(KIND=8) :: P(2*COL)
              INTEGER(KIND=4) :: INFO
            END SUBROUTINE BMV
          END INTERFACE 
        END MODULE BMV__genmod
