(define (problem test-grounder)
  (:domain test-grounder-domain)
  (:objects obj0 obj1 obj2 obj3 obj4)
  (:init (p1 obj0 obj1) (p2 obj1) (p1 obj1 obj0) )
  (:goal (p1 obj3 obj4)) ;;; Just a dummy goal for now.
)
