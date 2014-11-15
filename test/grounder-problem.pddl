(define (problem test-grounder)
  (:domain test-grounder-domain)
  (:objects obj1 obj2 obj3 obj4 obj5)
  (:init (p1 obj1 obj2) (p1 obj3 obj4) (p2 obj5))
  (:goal (p1 obj4 obj5)) ;;; Just a dummy goal for now.
)
