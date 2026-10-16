#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "config.h"

typedef struct {
    const char *current;
    int error;
    int division_by_zero;
} ExpressionParser;

static double parse_expression(ExpressionParser *parser);

static double parse_factor(ExpressionParser *parser) {
    char *end;
    double value;

    while (isspace((unsigned char)*parser->current))
        parser->current++;

    if (*parser->current == '+' || *parser->current == '-') {
        int negative = *parser->current == '-';
        parser->current++;
        value = parse_factor(parser);
        return negative ? -value : value;
    }

    if (*parser->current == '(') {
        parser->current++;
        value = parse_expression(parser);
        while (isspace((unsigned char)*parser->current))
            parser->current++;
        if (*parser->current != ')')
            parser->error = 1;
        else
            parser->current++;
        return value;
    }

    value = strtod(parser->current, &end);
    if (end == parser->current) {
        parser->error = 1;
        return 0.0;
    }

    parser->current = end;
    return value;
}

static double parse_term(ExpressionParser *parser) {
    double value = parse_factor(parser);

    while (!parser->error) {
        char operation;
        double next_value;

        while (isspace((unsigned char)*parser->current))
            parser->current++;

        operation = *parser->current;
        if (operation != '*' && operation != '/')
            break;

        parser->current++;
        next_value = parse_factor(parser);

        if (operation == '*') {
            value *= next_value;
        } else if (next_value == 0.0) {
            parser->division_by_zero = 1;
            parser->error = 1;
        } else {
            value /= next_value;
        }
    }

    return value;
}

static double parse_expression(ExpressionParser *parser) {
    double value = parse_term(parser);

    while (!parser->error) {
        char operation;
        double next_value;

        while (isspace((unsigned char)*parser->current))
            parser->current++;

        operation = *parser->current;
        if (operation != '+' && operation != '-')
            break;

        parser->current++;
        next_value = parse_term(parser);

        if (operation == '+')
            value += next_value;
        else
            value -= next_value;
    }

    return value;
}

int evaluate_expression(const char *input, double *result, int *division_by_zero) {
    ExpressionParser parser = { input, 0, 0 };

    *result = parse_expression(&parser);

    while (isspace((unsigned char)*parser.current))
        parser.current++;

    *division_by_zero = parser.division_by_zero;
    return !parser.error && *parser.current == '\0';
}

static void calculator_expression(void) {
    char input[MAX_INPUT];

    printf("Calculator\n");
    printf("Enter function (q for exit).\n");

    while (1) {
        double result;
        int division_by_zero;

        printf("calc> ");
        if (fgets(input, sizeof(input), stdin) == NULL)
            return;

        input[strcspn(input, "\n")] = '\0';

        if (strcmp(input, "q") == 0 || strcmp(input, "Q") == 0 || strcmp(input, "quit") == 0)
            return;

        if (input[0] == '\0')
            continue;

        if (evaluate_expression(input, &result, &division_by_zero)) {
            printf("= %.10g\n", result);
        } else if (division_by_zero) {
            printf("[ERROR]: dividing by 0 is not possible.\n");
        } else {
            printf("[ERROR]: bad input. Use numbers and operators + - * /.\n");
        }
    }
}

void calculator(void) {
    calculator_expression();
}
