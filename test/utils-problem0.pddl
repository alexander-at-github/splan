(define (problem test-utils)
  (:domain test-utils-domain)
  (:objects obj0 obj1 obj2 obj3)
  (:init (p1 obj0)) ;;; Just a dummy state for now.
  (:goal (and (p1 obj0)
              (p2 obj0 obj1)
              (not (p2 obj0 obj1))
              (not (p2 const0 obj1))))
)
