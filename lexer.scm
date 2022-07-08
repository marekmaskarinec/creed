(module (creed lexer) *
	(import
		scheme
		srfi-152
		(chicken base)
		(chicken format)
		(creed types))

	(define (lex input loc)
		;; decides the token type based on the first type
		(define (decide char)
			(cond
				((and (char>=? char #\0) (char<=? char #\9)) 'number)
				((char-whitespace? char) 'whitespace)
				((char=? char #\{) 'group-begin)
				((char=? char #\}) 'group-end)
				((char=? char #\") 'string)
				(else 'ident)))

		(define (walk-while input while)
			(define (do-walk input while)
				(if (while input)
					(cons (car input) (do-walk (cdr input) while))

					(list)))

			(list->string (do-walk (string->list input) while)))

		(if (= (string-length input) 0)
			(list)

			(let ((char (string-ref input 0)))
				(if (eq? (decide char) 'whitespace)
					(lex
						(substring input 1)
						(make-location
							(+ (location-lno loc) (if (eq? char #\newline) 1 0))
							(if (eq? char #\newline) 0 (+ (location-cno loc) 1))
							(location-filename loc)))
      
					(let
						((token
							(make-token
								(case (decide char)
									((number) (walk-while
										input
										(lambda (i)
											(eq? (decide (car i)) 'number))))
      
									((group-begin) (substring input 0 1))
      
									((group-end) (substring input 0 1))
      
									((string)
									 	(string-append
											"\""
										 	(walk-while
												(substring input 1)
												(lambda (i)
													(not (eq? (decide (car i)) 'string))))
											"\""))
      
									((ident) (walk-while
										input
										(lambda (i)
											(eq? (decide (car i)) 'ident)))))
      
								(decide char) loc)))
      
						(cons
							token
							(lex
								(substring
									input
									(string-length (token-value token)))
								(make-location
									(location-lno loc)
									(+
										(location-cno loc)
										(string-length (token-value token)))
									(location-filename loc)))))))))

	(define (convert-tokens tokens)
		(define (convert-value value type)
			(case type
				((number) (string->number value))
				((string) (substring value 1 (- (string-length value) 1)))
				((group-begin group-end) value)
				((ident) (string->symbol value))))

		(map
			(lambda (tok)
				(make-token
					(convert-value (token-value tok) (token-type tok))
					(token-type tok)
					(token-location tok)))
			tokens)))
