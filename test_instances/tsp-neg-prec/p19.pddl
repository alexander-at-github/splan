


(define (problem tsp-20)
(:domain tsp)
(:objects p1 p2 p3 p4 p5 p6 p7 p8 p9 p10 p11 p12 p13 p14 p15 p16 p17 p18 p19 p20 )
(:init
(at p1)
(connected p1 p2)
(connected p2 p1)
(connected p1 p4)
(connected p4 p1)
(connected p1 p5)
(connected p5 p1)
(connected p1 p7)
(connected p7 p1)
(connected p1 p11)
(connected p11 p1)
(connected p1 p14)
(connected p14 p1)
(connected p1 p15)
(connected p15 p1)
(connected p1 p18)
(connected p18 p1)
(connected p1 p19)
(connected p19 p1)
(connected p3 p2)
(connected p2 p3)
(connected p3 p5)
(connected p5 p3)
(connected p3 p7)
(connected p7 p3)
(connected p3 p8)
(connected p8 p3)
(connected p3 p9)
(connected p9 p3)
(connected p3 p11)
(connected p11 p3)
(connected p3 p14)
(connected p14 p3)
(connected p3 p15)
(connected p15 p3)
(connected p3 p18)
(connected p18 p3)
(connected p3 p20)
(connected p20 p3)
(connected p5 p2)
(connected p2 p5)
(connected p5 p3)
(connected p3 p5)
(connected p5 p4)
(connected p4 p5)
(connected p5 p6)
(connected p6 p5)
(connected p5 p9)
(connected p9 p5)
(connected p5 p11)
(connected p11 p5)
(connected p5 p13)
(connected p13 p5)
(connected p5 p15)
(connected p15 p5)
(connected p5 p18)
(connected p18 p5)
(connected p5 p19)
(connected p19 p5)
(connected p5 p20)
(connected p20 p5)
(connected p7 p2)
(connected p2 p7)
(connected p7 p4)
(connected p4 p7)
(connected p7 p5)
(connected p5 p7)
(connected p7 p9)
(connected p9 p7)
(connected p7 p10)
(connected p10 p7)
(connected p7 p11)
(connected p11 p7)
(connected p7 p13)
(connected p13 p7)
(connected p7 p15)
(connected p15 p7)
(connected p7 p16)
(connected p16 p7)
(connected p7 p18)
(connected p18 p7)
(connected p9 p3)
(connected p3 p9)
(connected p9 p5)
(connected p5 p9)
(connected p9 p7)
(connected p7 p9)
(connected p9 p8)
(connected p8 p9)
(connected p9 p11)
(connected p11 p9)
(connected p9 p13)
(connected p13 p9)
(connected p9 p20)
(connected p20 p9)
(connected p11 p2)
(connected p2 p11)
(connected p11 p3)
(connected p3 p11)
(connected p11 p4)
(connected p4 p11)
(connected p11 p6)
(connected p6 p11)
(connected p11 p7)
(connected p7 p11)
(connected p11 p8)
(connected p8 p11)
(connected p11 p10)
(connected p10 p11)
(connected p11 p12)
(connected p12 p11)
(connected p11 p13)
(connected p13 p11)
(connected p11 p14)
(connected p14 p11)
(connected p11 p16)
(connected p16 p11)
(connected p11 p19)
(connected p19 p11)
(connected p13 p6)
(connected p6 p13)
(connected p13 p7)
(connected p7 p13)
(connected p13 p11)
(connected p11 p13)
(connected p13 p12)
(connected p12 p13)
(connected p13 p14)
(connected p14 p13)
(connected p13 p15)
(connected p15 p13)
(connected p13 p20)
(connected p20 p13)
(connected p15 p3)
(connected p3 p15)
(connected p15 p4)
(connected p4 p15)
(connected p15 p5)
(connected p5 p15)
(connected p15 p6)
(connected p6 p15)
(connected p15 p8)
(connected p8 p15)
(connected p15 p9)
(connected p9 p15)
(connected p15 p10)
(connected p10 p15)
(connected p15 p11)
(connected p11 p15)
(connected p15 p14)
(connected p14 p15)
(connected p15 p16)
(connected p16 p15)
(connected p15 p17)
(connected p17 p15)
(connected p15 p18)
(connected p18 p15)
(connected p15 p19)
(connected p19 p15)
(connected p17 p2)
(connected p2 p17)
(connected p17 p3)
(connected p3 p17)
(connected p17 p5)
(connected p5 p17)
(connected p17 p9)
(connected p9 p17)
(connected p17 p11)
(connected p11 p17)
(connected p17 p16)
(connected p16 p17)
(connected p17 p18)
(connected p18 p17)
(connected p17 p20)
(connected p20 p17)
(connected p19 p2)
(connected p2 p19)
(connected p19 p4)
(connected p4 p19)
(connected p19 p7)
(connected p7 p19)
(connected p19 p9)
(connected p9 p19)
(connected p19 p13)
(connected p13 p19)
(connected p19 p16)
(connected p16 p19)
)
(:goal
(and
(visited p1)
(visited p2)
(visited p3)
(visited p4)
(visited p5)
(visited p6)
(visited p7)
(visited p8)
(visited p9)
(visited p10)
(visited p11)
(visited p12)
(visited p13)
(visited p14)
(visited p15)
(visited p16)
(visited p17)
(visited p18)
(visited p19)
(visited p20)
)
)
)

