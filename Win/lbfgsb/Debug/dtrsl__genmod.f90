        !COMPILER-GENERATED INTERFACE MODULE: Sun Apr 29 10:47:50 2012
        MODULE DTRSL__genmod
          INTERFACE 
            SUBROUTINE DTRSL(T,LDT,N,B,JOB,INFO)
              INTEGER(KIND=4) :: LDT
              REAL(KIND=8) :: T(LDT,*)
              INTEGER(KIND=4) :: N
              REAL(KIND=8) :: B(*)
              INTEGER(KIND=4) :: JOB
              INTEGER(KIND=4) :: INFO
            END SUBROUTINE DTRSL
          END INTERFACE 
        END MODULE DTRSL__genmod
