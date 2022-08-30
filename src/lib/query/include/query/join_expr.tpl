{
	"JoinExpr": {
		"jointype": "JOIN_INNER",
		"larg": {
			"RangeVar": {
				{{#ALIAS}}"alias": {
					"aliasname": "{{TABLE_ALIAS}}"
				},{{/ALIAS}}
				"inh": true,
				"relname": "{{TABLE_NAME}}",
				"relpersistence": "p"
			}
		},
		"quals": {
			"BoolExpr": {
				"args": [
					{
						"A_Expr": {
							"kind": "AEXPR_OP",
							"lexpr": {
								"ColumnRef": {
									"fields": [
										{
											"String": {
												"str": "{{TABLE_REF}}"
											}
										},
										{
											"String": {
												"str": "id"
											}
										}
									]
								}
							},
							"name": [
								{
									"String": {
										"str": "="
									}
								}
							],
							"rexpr": {
								"ColumnRef": {
									"fields": [
										{
											"String": {
												"str": "{{TABLE_REF}}__acl__"
											}
										},
										{
											"String": {
												"str": "{{TABLE_NAME}}_id"
											}
										}
									]
								}
							}
						}
					},
					{
						"A_Expr": {
							"kind": "AEXPR_OP",
							"lexpr": {
								"ColumnRef": {
									"fields": [
										{
											"String": {
												"str": "{{TABLE_REF}}__acl__"
											}
										},
										{
											"String": {
												"str": "perm_name"
											}
										}
									]
								}
							},
							"name": [
								{
									"String": {
										"str": "="
									}
								}
							],
							"rexpr": {
								"A_Const": {
									"val": {
										"Integer": {
											"ival": {{PERM_ID}}
										}
									}
								}
							}
						}
					},
					{
						"A_Expr": {
							"kind": "AEXPR_OP",
							"lexpr": {
								"ColumnRef": {
									"fields": [
										{
											"String": {
												"str": "{{TABLE_REF}}_acl"
											}
										},
										{
											"String": {
												"str": "principal"
											}
										}
									]
								}
							},
							"name": [
								{
									"String": {
										"str": "="
									}
								}
							],
							"rexpr": {
								"A_Const": {
									"val": {
										"String": {
											"str": "{{PRINCIPAL}}"
										}
									}
								}
							}
						}
					}
				],
				"boolop": "AND_EXPR"
			}
		},
		"rarg": {
			"RangeVar": {
				{{#ALIAS}}"alias": {
					"aliasname": "{{TABLE_ALIAS}}_acl"
				},{{/ALIAS}}
				"inh": true,
				"relname": "{{TABLE_NAME}}_acl",
				"relpersistence": "p"
			}
		}
	}
}