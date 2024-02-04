#include <iostream>
#include <string.h>
#include <vector>
#include <memory>

// Перечисление типов токенов
enum SyntaxKind {
    NumberToken,
    WhitespaceToken,
    PlusToken,
    MinusToken,
    StarToken,
    SlashToken,
    OpenParantesisToken,
    CloseParantesisToken,
    EndOfFileToken,

    NumberExpression,
    BinaryExpression,

    BadToken
};

class SyntaxNode {
public:
    virtual SyntaxKind GetKind() const = 0;
    virtual std::vector<std::unique_ptr<SyntaxNode>> GetChildren() const = 0;
    virtual std::unique_ptr<SyntaxNode> clone() const = 0;
    virtual ~SyntaxNode() = default;
};

class SyntaxToken : public SyntaxNode {
public:
    SyntaxToken(SyntaxKind kind, int position, std::string text, int value = 0)
        : Kind(kind), Position(position), Text(text), Value(value) {}

    SyntaxToken() {}

    SyntaxKind GetKind() const override {
        return Kind;
    }

    SyntaxKind Kind;

    int Position;
    std::string Text;
    int Value;

    std::vector<std::unique_ptr<SyntaxNode>> GetChildren() const override {
        return {};
    }

    std::unique_ptr<SyntaxNode> clone() const override {
        return std::make_unique<SyntaxToken>(*this);
    } 
};

class ExpressionSyntax : public SyntaxNode {
public:
    SyntaxKind GetKind() const override {
        return SyntaxKind::NumberExpression;
    }

    std::vector<std::unique_ptr<SyntaxNode>> GetChildren() const override {
        return {};
    }

    std::unique_ptr<SyntaxNode> clone() const override {
        return std::make_unique<ExpressionSyntax>(*this);
    }
};

class Lexer {
public:
    Lexer(std::string text) : _text(text), _position(0), _current(0) {}

    SyntaxToken NextToken() {
        if (_position >= _text.length()) {
            return SyntaxToken(SyntaxKind::EndOfFileToken, _position, "\0");
        }

        if (isdigit(getCurrent())) {
            int start = _position;

            while (isdigit(getCurrent())) {
                Next();
            }

            int length = _position - start;
            std::string text = _text.substr(start, length);
            return SyntaxToken(SyntaxKind::NumberToken, start, text, std::stoi(text));
        }

        if (isspace(getCurrent())) {
            int start = _position;

            while (isspace(getCurrent())) {
                Next();
            }

            int length = _position - start;
            std::string text = _text.substr(start, length);
            return SyntaxToken(SyntaxKind::WhitespaceToken, start, text);
        }

        if (getCurrent() == '+') {
            return SyntaxToken(SyntaxKind::PlusToken, _position++, "+");
        } else if (getCurrent() == '-') {
            return SyntaxToken(SyntaxKind::MinusToken, _position++, "-");
        } else if (getCurrent() == '*') {
            return SyntaxToken(SyntaxKind::StarToken, _position++, "*");
        } else if (getCurrent() == '/') {
            return SyntaxToken(SyntaxKind::SlashToken, _position++, "/");
        } else if (getCurrent() == '(') {
            return SyntaxToken(SyntaxKind::OpenParantesisToken, _position++, "(");
        } else if (getCurrent() == ')') {
            return SyntaxToken(SyntaxKind::CloseParantesisToken, _position++, ")");
        }

        return SyntaxToken(SyntaxKind::BadToken, _position++, _text.substr(_position - 1, 1));
    }

private:
    std::string _text;
    int _position;
    char _current;

    char getCurrent() {
        if (_position >= _text.length()) {
            return '\0';
        }
        return _text[_position];
    }

    void Next() {
        _position++;
    }
};

class NumberExpressionSyntax : public ExpressionSyntax {
public:
    NumberExpressionSyntax(SyntaxToken* numberToken)
        : NumberToken(std::make_unique<SyntaxToken>(*numberToken)) {}

    SyntaxKind GetKind() const override {
        return SyntaxKind::NumberExpression;
    }

    std::vector<std::unique_ptr<SyntaxNode>> GetChildren() const override {
        std::vector<std::unique_ptr<SyntaxNode>> numberVector;
        numberVector.push_back(NumberToken->clone());
        return numberVector;
    }

    std::unique_ptr<SyntaxNode> clone() const override {
        return std::make_unique<NumberExpressionSyntax>(*this);
    }

private:
    std::unique_ptr<SyntaxToken> NumberToken;
};

class BinaryExpressionSyntax : public ExpressionSyntax {
public:
    BinaryExpressionSyntax(
        std::unique_ptr<ExpressionSyntax> left,
        SyntaxToken* operatorToken,
        std::unique_ptr<ExpressionSyntax> right)
        : Left(std::move(left)),
          OperatorToken(std::make_unique<SyntaxToken>(*operatorToken)),
          Right(std::move(right)) {}

    SyntaxKind GetKind() const override {
        return SyntaxKind::BinaryExpression;
    }

    std::vector<std::unique_ptr<SyntaxNode>> GetChildren() const override {
        std::vector<std::unique_ptr<SyntaxNode>> binaryExpressionVector;
        binaryExpressionVector.push_back(Left->clone());
        binaryExpressionVector.push_back(OperatorToken->clone());
        binaryExpressionVector.push_back(Right->clone());
        return binaryExpressionVector;
    }

    std::unique_ptr<SyntaxNode> clone() const override {
        return std::make_unique<BinaryExpressionSyntax>(
            std::move(Left->clone()),
            OperatorToken->clone(),
            std::move(Right->clone())
        );
    }

private:
    std::unique_ptr<ExpressionSyntax> Left;
    std::unique_ptr<SyntaxToken> OperatorToken;
    std::unique_ptr<ExpressionSyntax> Right;
};

class Parser {
public:
    Parser(std::string text) : _tokens(), _position(0) {
        Lexer lexer = Lexer(text);
        SyntaxToken token;
        do {
            token = lexer.NextToken();

            if (token.Kind != SyntaxKind::WhitespaceToken &&
                token.Kind != SyntaxKind::BadToken) {
                _tokens.push_back(std::make_unique<SyntaxToken>(token));
            }

        } while (token.Kind != SyntaxKind::EndOfFileToken);
    }

    std::unique_ptr<ExpressionSyntax> Parse() {
        auto left = ParsePrimaryExpression();

        while (getCurrent().GetKind() == SyntaxKind::PlusToken ||
               getCurrent().GetKind() == SyntaxKind::MinusToken) {
            SyntaxToken operatorToken = NextToken();
            auto right = ParsePrimaryExpression();
            left = std::make_unique<BinaryExpressionSyntax>(std::move(left), &operatorToken, std::move(right));
        }

        return left;
    }

    std::unique_ptr<ExpressionSyntax> ParsePrimaryExpression() {
        SyntaxToken numberToken = Match(SyntaxKind::NumberToken);
        return std::make_unique<NumberExpressionSyntax>(&numberToken);
    }

private:
    std::vector<std::unique_ptr<SyntaxToken>> _tokens;
    int _position;

    SyntaxToken getCurrent() {
        return Peek(0);
    }

    SyntaxToken NextToken() {
        SyntaxToken current = getCurrent();
        _position++;
        return current;
    }

    SyntaxToken Match(SyntaxKind kind) {
        if (getCurrent().GetKind() == kind)
            return NextToken();
        return SyntaxToken(kind, getCurrent().Position, "");
    }

    SyntaxToken Peek(int offset) {
        int index = _position + offset;
        if (index >= _tokens.size())
            return *_tokens.back();

        return *_tokens[index];
    }
};

void PrettyPrint(const SyntaxNode& node, std::string indent = "") {
    std::cout << indent << static_cast<int>(node.GetKind());

    if (auto t = dynamic_cast<const SyntaxToken*>(&node)) {
        std::cout << " " << t->Value;
    }

    indent += "    ";

    for (const auto& child : node.GetChildren()) {
        PrettyPrint(*child, indent);
    }
}

int main() {
    while (true) {
        std::cout << "> ";
        std::string line;
        std::getline(std::cin, line);

        if (line.empty())
            break;

        Parser parser = Parser(line);

        try {
            std::unique_ptr<ExpressionSyntax> expression = parser.Parse();
            PrettyPrint(*expression);
            std::cout << std::endl;
        } catch (const std::exception& ex) {
            std::cerr << "Error: " << ex.what() << std::endl;
        }
    }

    return 0;
}
