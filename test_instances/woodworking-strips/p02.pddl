; woodworking task with 6 parts and 140% wood
; Machines:
;   1 grinder
;   1 glazer
;   1 immersion-varnisher
;   1 planer
;   1 highspeed-saw
;   1 spray-varnisher
;   1 saw
; random seed: 887881

(define (problem wood-prob)
  (:domain woodworking)
  (:objects
    grinder0 - grinder
    glazer0 - glazer
    immersion-varnisher0 - immersion-varnisher
    planer0 - planer
    highspeed-saw0 - highspeed-saw
    spray-varnisher0 - spray-varnisher
    saw0 - saw
    black blue mauve red - acolour
    walnut mahogany - awood
    p0 p1 p2 p3 p4 p5 - part
    b0 - board
    s0 s1 s2 s3 s4 s5 s6 - aboardsize
  )
  (:init
    (grind-treatment-change varnished colourfragments)
    (grind-treatment-change glazed untreated)
    (grind-treatment-change untreated untreated)
    (grind-treatment-change colourfragments untreated)
    (is-smooth smooth)
    (is-smooth verysmooth)
    (= (total-cost) 0)
    (boardsize-successor s0 s1)
    (boardsize-successor s1 s2)
    (boardsize-successor s2 s3)
    (boardsize-successor s3 s4)
    (boardsize-successor s4 s5)
    (boardsize-successor s5 s6)
    (has-colour glazer0 blue)
    (has-colour immersion-varnisher0 mauve)
    (has-colour immersion-varnisher0 black)
    (empty highspeed-saw0)
    (has-colour spray-varnisher0 mauve)
    (has-colour spray-varnisher0 black)
    (available p0)
    (colour p0 natural)
    (wood p0 walnut)
    (surface-condition p0 verysmooth)
    (treatment p0 glazed)
    (goalsize p0 small)
    (= (spray-varnish-cost p0) 5)
    (= (glaze-cost p0) 10)
    (= (grind-cost p0) 15)
    (= (plane-cost p0) 10)
    (unused p1)
    (goalsize p1 medium)
    (= (spray-varnish-cost p1) 10)
    (= (glaze-cost p1) 15)
    (= (grind-cost p1) 30)
    (= (plane-cost p1) 20)
    (available p2)
    (colour p2 black)
    (wood p2 mahogany)
    (surface-condition p2 rough)
    (treatment p2 glazed)
    (goalsize p2 large)
    (= (spray-varnish-cost p2) 15)
    (= (glaze-cost p2) 20)
    (= (grind-cost p2) 45)
    (= (plane-cost p2) 30)
    (unused p3)
    (goalsize p3 medium)
    (= (spray-varnish-cost p3) 10)
    (= (glaze-cost p3) 15)
    (= (grind-cost p3) 30)
    (= (plane-cost p3) 20)
    (available p4)
    (colour p4 black)
    (wood p4 mahogany)
    (surface-condition p4 verysmooth)
    (treatment p4 varnished)
    (goalsize p4 large)
    (= (spray-varnish-cost p4) 15)
    (= (glaze-cost p4) 20)
    (= (grind-cost p4) 45)
    (= (plane-cost p4) 30)
    (available p5)
    (colour p5 mauve)
    (wood p5 walnut)
    (surface-condition p5 rough)
    (treatment p5 glazed)
    (goalsize p5 small)
    (= (spray-varnish-cost p5) 5)
    (= (glaze-cost p5) 10)
    (= (grind-cost p5) 15)
    (= (plane-cost p5) 10)
    (boardsize b0 s6)
    (wood b0 walnut)
    (surface-condition b0 smooth)
    (available b0)
  )
  (:goal
    (and
      (available p0)
      (colour p0 black)
      (treatment p0 varnished)
      (available p1)
      (wood p1 walnut)
      (surface-condition p1 smooth)
      (treatment p1 glazed)
      (available p2)
      (colour p2 blue)
      (surface-condition p2 verysmooth)
      (treatment p2 glazed)
      (available p3)
      (surface-condition p3 smooth)
      (treatment p3 varnished)
      (available p4)
      (colour p4 mauve)
      (wood p4 mahogany)
      (surface-condition p4 smooth)
      (treatment p4 varnished)
      (available p5)
      (surface-condition p5 verysmooth)
      (treatment p5 varnished)
    )
  )
  (:metric minimize (total-cost))
)
