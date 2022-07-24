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
		(state-stack-set! state (cons val (state-stack state))))

	(define (apply-symbol! state symbol)
		(define (symbol-defined? state symbol)
			(hash-table-exists? (state-symbols state) symbol))
  
		(define (state-symbol-ref state symbol)
			(hash-table-ref (state-symbols state) symbol))

		(if (symbol-defined? state symbol)
			(apply! state (state-symbol-ref state symbol))
			(abort (make-crerror 'undefined-symbol))))

	(define (apply! state tokens)
		(if (null? tokens)
			state

			(begin
				(case (token-type (car tokens))
					((number string group) (creed-push! state (token-value (car tokens))))
					((builtin) ((token-value (car tokens)) state (token-location (car tokens))))
					((symbol) (apply-symbol! state (car tokens))))

				(apply! state (cdr tokens))))))
