(module (creed runtime)
	(apply!)
	(import
		scheme
		srfi-12
		srfi-69
		utf8
		(chicken base)
		(creed types))

	(define (creed-push! state val)
		(state-stack-set! (cons val (state-stack state))))


	(define (apply-ident! state ident)
		(define (ident-defined? state ident)
			(hash-table-exists? (state-symbols state) ident))
  
		(define (state-ident-ref state ident)
			(hash-table-ref (state-symbols state) ident))

		(if (ident-defined? state ident)
			(apply! state (state-ident-ref state ident))
			(abort (make-crerror 'undefined-ident))))

	(define (apply! state tokens)
		(if (null? tokens)
			state

			(begin
				(case (token-type (car tokens))
					((number string group) (creed-push! state (token-value (car tokens))))
					((builtin) ((token-value (car tokens)) state (car tokens)))
					((ident) (apply-ident! state (car tokens))))

				(apply! state (cdr tokens))))))
