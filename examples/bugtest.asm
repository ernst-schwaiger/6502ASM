            ;FOO = $12
            .ORG $1000 
routine1:   TAY
            BMI farAway
            .ORG $2000 
farAway:    RTS
