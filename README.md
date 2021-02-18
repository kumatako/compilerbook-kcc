# compilerbook-kcc
参考: https://www.sigbus.info/compilerbook

## 文法
	program		= statement*
	statement	= expr ";"
				| "if" "(" expr ")" statement ("else" statement)?
				| "return" expr ";"
	expr		= assign
	assign		= equality ("=" assign)?
	equality	= relational ("==" relational | "!=" relational)*
	relational	= add ("<" add | "<=" add | ">" add | ">=" add)*
	add			= mul ("+" mul | "-" mul)*
	mul			= unary ("*" unary | "/" unary)*
	unary		= ("+" | "-")? primary
	primary		= num | ident | "(" expr ")"


