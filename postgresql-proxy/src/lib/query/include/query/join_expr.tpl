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
												"str": "{{TABLE_REF}}__acls__"
											}
										},
										{
											"String": {
												"str": "_id"
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
												"str": "{{TABLE_REF}}__acls__"
											}
										},
										{
											"String": {
												"str": "_principal"
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
					},
					{
						"A_Expr": {
							"kind": "AEXPR_OP",
							"lexpr": {
								"ColumnRef": {
									"fields": [
										{
											"String": {
												"str": "{{TABLE_REF}}__acls__"
											}
										},
										{
											"String": {
												"str": "_operation"
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
					}
				],
				"boolop": "AND_EXPR"
			}
		},
		"rarg": {
			"RangeVar": {
				{{#ALIAS}}"alias": {
					"aliasname": "{{TABLE_ALIAS}}__acls__"
				},{{/ALIAS}}
				"inh": true,
				"relname": "{{TABLE_NAME}}__acls__",
				"relpersistence": "p"
			}
		}
	}
}