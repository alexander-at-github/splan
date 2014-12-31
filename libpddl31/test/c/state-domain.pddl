(define (domain test-state-domain)
  ;(:requirements)

  ;(:types)

  (:constants const0 const1)

  (:predicates (p0 ?a)
               (p1 ?a ?b)
               (p2 ?a ?b ?c)
               (p3 ?a ?b ?c))

  (:action action0
    :parameters (?a ?b ?c)
    :precondition ()
    :effect (p2 ?a ?b ?c))
)
