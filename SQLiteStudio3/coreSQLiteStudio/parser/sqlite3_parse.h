#define TK3_ILLEGAL                          1
#define TK3_COMMENT                          2
#define TK3_SPACE                            3
#define TK3_ID                               4
#define TK3_ABORT                            5
#define TK3_ACTION                           6
#define TK3_AFTER                            7
#define TK3_ANALYZE                          8
#define TK3_ASC                              9
#define TK3_ATTACH                          10
#define TK3_BEFORE                          11
#define TK3_BEGIN                           12
#define TK3_BY                              13
#define TK3_CASCADE                         14
#define TK3_CAST                            15
#define TK3_COLUMNKW                        16
#define TK3_CONFLICT                        17
#define TK3_DATABASE                        18
#define TK3_DEFERRED                        19
#define TK3_DESC                            20
#define TK3_DETACH                          21
#define TK3_EACH                            22
#define TK3_END                             23
#define TK3_EXCLUSIVE                       24
#define TK3_EXPLAIN                         25
#define TK3_FAIL                            26
#define TK3_FOR                             27
#define TK3_IGNORE                          28
#define TK3_IMMEDIATE                       29
#define TK3_INDEXED                         30
#define TK3_INITIALLY                       31
#define TK3_INSTEAD                         32
#define TK3_LIKE_KW                         33
#define TK3_MATCH                           34
#define TK3_NO                              35
#define TK3_PLAN                            36
#define TK3_QUERY                           37
#define TK3_KEY                             38
#define TK3_OF                              39
#define TK3_OFFSET                          40
#define TK3_PRAGMA                          41
#define TK3_RAISE                           42
#define TK3_RECURSIVE                       43
#define TK3_RELEASE                         44
#define TK3_REPLACE                         45
#define TK3_RESTRICT                        46
#define TK3_ROW                             47
#define TK3_ROLLBACK                        48
#define TK3_SAVEPOINT                       49
#define TK3_TEMP                            50
#define TK3_TRIGGER                         51
#define TK3_VACUUM                          52
#define TK3_VIEW                            53
#define TK3_VIRTUAL                         54
#define TK3_WITH                            55
#define TK3_WITHOUT                         56
#define TK3_REINDEX                         57
#define TK3_RENAME                          58
#define TK3_CTIME_KW                        59
#define TK3_IF                              60
#define TK3_ANY                             61
#define TK3_OR                              62
#define TK3_AND                             63
#define TK3_NOT                             64
#define TK3_IS                              65
#define TK3_BETWEEN                         66
#define TK3_IN                              67
#define TK3_ISNULL                          68
#define TK3_NOTNULL                         69
#define TK3_NE                              70
#define TK3_EQ                              71
#define TK3_GT                              72
#define TK3_LE                              73
#define TK3_LT                              74
#define TK3_GE                              75
#define TK3_ESCAPE                          76
#define TK3_BITAND                          77
#define TK3_BITOR                           78
#define TK3_LSHIFT                          79
#define TK3_RSHIFT                          80
#define TK3_PLUS                            81
#define TK3_MINUS                           82
#define TK3_STAR                            83
#define TK3_SLASH                           84
#define TK3_REM                             85
#define TK3_CONCAT                          86
#define TK3_COLLATE                         87
#define TK3_BITNOT                          88
#define TK3_SEMI                            89
#define TK3_TRANSACTION                     90
#define TK3_ID_TRANS                        91
#define TK3_COMMIT                          92
#define TK3_TO                              93
#define TK3_CREATE                          94
#define TK3_TABLE                           95
#define TK3_LP                              96
#define TK3_RP                              97
#define TK3_AS                              98
#define TK3_DOT                             99
#define TK3_ID_TAB_NEW                     100
#define TK3_ID_DB                          101
#define TK3_CTX_ROWID_KW                   102
#define TK3_EXISTS                         103
#define TK3_COMMA                          104
#define TK3_ID_COL_NEW                     105
#define TK3_STRING                         106
#define TK3_JOIN_KW                        107
#define TK3_ID_COL_TYPE                    108
#define TK3_CONSTRAINT                     109
#define TK3_DEFAULT                        110
#define TK3_NULL                           111
#define TK3_PRIMARY                        112
#define TK3_UNIQUE                         113
#define TK3_CHECK                          114
#define TK3_REFERENCES                     115
#define TK3_ID_CONSTR                      116
#define TK3_ID_COLLATE                     117
#define TK3_ID_TAB                         118
#define TK3_INTEGER                        119
#define TK3_FLOAT                          120
#define TK3_BLOB                           121
#define TK3_AUTOINCR                       122
#define TK3_ON                             123
#define TK3_INSERT                         124
#define TK3_DELETE                         125
#define TK3_UPDATE                         126
#define TK3_ID_FK_MATCH                    127
#define TK3_SET                            128
#define TK3_DEFERRABLE                     129
#define TK3_FOREIGN                        130
#define TK3_DROP                           131
#define TK3_ID_VIEW_NEW                    132
#define TK3_ID_VIEW                        133
#define TK3_UNION                          134
#define TK3_ALL                            135
#define TK3_EXCEPT                         136
#define TK3_INTERSECT                      137
#define TK3_SELECT                         138
#define TK3_VALUES                         139
#define TK3_DISTINCT                       140
#define TK3_ID_ALIAS                       141
#define TK3_FROM                           142
#define TK3_USING                          143
#define TK3_JOIN                           144
#define TK3_ID_JOIN_OPTS                   145
#define TK3_ID_IDX                         146
#define TK3_ORDER                          147
#define TK3_GROUP                          148
#define TK3_HAVING                         149
#define TK3_LIMIT                          150
#define TK3_WHERE                          151
#define TK3_ID_COL                         152
#define TK3_INTO                           153
#define TK3_VARIABLE                       154
#define TK3_CASE                           155
#define TK3_ID_FN                          156
#define TK3_ID_ERR_MSG                     157
#define TK3_WHEN                           158
#define TK3_THEN                           159
#define TK3_ELSE                           160
#define TK3_INDEX                          161
#define TK3_ID_IDX_NEW                     162
#define TK3_ID_PRAGMA                      163
#define TK3_ID_TRIG_NEW                    164
#define TK3_ID_TRIG                        165
#define TK3_ALTER                          166
#define TK3_ADD                            167
