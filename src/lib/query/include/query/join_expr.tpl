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
												"str": "id"
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
												"str": "operation"
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
											"str": "{{OPERATION}}"
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
												"str": "{{TABLE_REF}}__acl__"
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
                                "SQLValueFunction": {
                                    "op": "SVFOP_CURRENT_USER",
                                    "typmod": -1
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
					"aliasname": "{{TABLE_ALIAS}}__acl__"
				},{{/ALIAS}}
				"inh": true,
				"relname": "{{TABLE_NAME}}__acl__",
				"relpersistence": "p"
			}
		}
	}
}