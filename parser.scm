(module (creed parser) *
	(import
		scheme
		srfi-12
		srfi-69
		srfi-152
		(chicken base)
		(chicken format)
		(creed types)
		(creed builtins))

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
				(if (and (not (null? input)) (while input))
					(cons (car input) (do-walk (cdr input) while))

					(list)))

			(list->string (do-walk (string->list input) while)))

		(define (lex-next input char)
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
						(eq? (decide (car i)) 'ident))))))

		(define (handle-whitespaces input char loc)
			(lex
				(substring input 1)
				(make-location
					(+ (location-lno loc) (if (eq? char #\newline) 1 0))
					(if (eq? char #\newline) 0 (+ (location-cno loc) 1))
					(location-filename loc))))

		(define (advance-location loc token)
			(make-location
				(location-lno loc)
				(+
					(location-cno loc)
					(string-length (token-value token)))
				(location-filename loc)))

		(define (advance-buffer input token)
			(substring
				input
				(string-length (token-value token))))

		(if (= (string-length input) 0)
			(list)

			(let ((char (string-ref input 0)))
				(if (eq? (decide char) 'whitespace)
					(handle-whitespaces input char loc)
      
					(let
						((token
							(make-token
     						(lex-next input char) 
								(decide char) loc)))
						(cons
							token
							(lex
								(advance-buffer input token)
								(advance-location loc token))))))))

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
			tokens))

	(define (generate-tree tokens)
		(define (collect-group group tokens)
			;; handle unterminated group
			(when (null? tokens)
				(abort (make-crerror
					'unterminated-group
					(make-location 0 0 ""))))

			(if (eq? (token-type (car tokens)) 'group-end)
				;; basecase
				(cons (reverse group) (generate-tree (cdr tokens)))
				;; else
				(collect-group (cons (car tokens) group) (cdr tokens))))

		(if (null? tokens)
			;; basecase
			(list)

			;; else
			(if (eq? (token-type (car tokens)) 'group-begin)
				;; found a group
				(let ((group (collect-group (list) (cdr tokens))))
					(cons
						(make-token (car group) 'group (token-location (car tokens)))
						(cdr group)))

				;; else
				(if (eq? (token-type (car tokens)) 'group-end)
					;; handle } without an open group
					(abort (make-crerror
						'unexpected-token
						(token-location (car tokens))))

					;; else
					(cons (car tokens) (generate-tree (cdr tokens)))))))

		(define (link-builtins tokens)
			(define (is-builtin? token)
				(and
					(eq? (token-type token) 'ident)
					(hash-table-exists? builtins (token-value token))))

			(define (link-builtin token)
				(make-token
					(hash-table-ref builtins (token-value token))
					'builtin
					(token-location token)))

			(if (null? tokens)
				(list)

				(cons
					(if (is-builtin? (car tokens))
						(link-builtin (car tokens))
						(car tokens))
					(link-builtins (cdr tokens)))))


	(define (parse text filename)
		(link-builtins (generate-tree (convert-tokens (lex text (make-location 0 0 filename)))))))
