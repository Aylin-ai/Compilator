#include <iostream>
#include <string.h>
#include <vector>
#include <memory>

//Перечесление типов токенов
enum SyntaxKind{
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

/*
Узел синтаксического дерева. От него исходят все остальные типы.
Содержит абстрактные методы для определения типа токена и для определения
тех узлов, которые исходят от нашего узла
*/
class SyntaxNode{
    public:
        virtual SyntaxKind GetKind() = 0;
        virtual std::vector<std::unique_ptr<SyntaxNode>> GetChildren() = 0;
};

/*
Класс, определяющий синтаксис токена. Содержит тип токена,
его позицию в тексте, его лексему и значение.
Переопределяет методы класса SyntaxNode
*/
class SyntaxToken : public SyntaxNode{
    public:
    //object value пока не реализуемо
        SyntaxToken(SyntaxKind kind, int position, std::string text, int value = 0){
            Kind = kind;
            Position = position;
            Text = text;
            Value = value;
        }
        SyntaxToken(){

        }

        //Тип токена
        SyntaxKind Kind;
        SyntaxKind GetKind() override {
            return Kind;
        }
        //Позиция токена в тексте
        int Position;
        //Текст токена
        std::string Text;
        //Значение токена
        int Value;

        std::vector<std::unique_ptr<SyntaxNode>> GetChildren() override {
            return {};  // возвращаем пустой вектор указателей
        }
};

/*
Класс, определяющий синтаксис выражения expression.
Является дочерним для класса SyntaxNode, определяющим узел в дереве.
*/
class ExpressionSyntax : public SyntaxNode{
    public:
        SyntaxKind GetKind() override {
            return SyntaxKind::NumberExpression; // Здесь должен быть соответствующий тип
        }

        std::vector<std::unique_ptr<SyntaxNode>> GetChildren() override {
            return {};  // Пока не имеет детей
        }
};

//Преобразует лексемы в токены
class Lexer{
    public:
        Lexer(std::string text){
            _text = text;
        }
    
        SyntaxToken NextToken(){
            /*
            Просматривает входной текст и выделяет токен в зависимости
            от того, с каким символом столкнулся
            Если столкнулся со сложной лексемой (число, пробелы),
            то проходит цикл и обнаруживает конец лексемы, вычисляя ее длину.
            Возвращает токен
            */


            if (_position >= _text.length()){
                return SyntaxToken(SyntaxKind::EndOfFileToken, _position, "\0");
            }

            if (isdigit(getCurrent())){
                int start = _position;

                while (isdigit(getCurrent()))
                {
                    Next();
                }

                int length = _position - start;
                std::string text = _text.substr(start, length);
                return SyntaxToken(SyntaxKind::NumberToken, start, text, std::stoi(text));
            }

            if (isspace(getCurrent())){
                int start = _position;

                while (isspace(getCurrent()))
                {
                    Next();
                }

                int length = _position - start;
                std::string text = _text.substr(start, length);
                return SyntaxToken(SyntaxKind::WhitespaceToken, start, text);
            }

            if (getCurrent() == '+'){
                return SyntaxToken(SyntaxKind::PlusToken, _position++, "+");
            } else if (getCurrent() == '-'){
                return SyntaxToken(SyntaxKind::MinusToken, _position++, "-");
            } else if (getCurrent() == '*'){
                return SyntaxToken(SyntaxKind::StarToken, _position++, "*");
            } else if (getCurrent() == '/'){
                return SyntaxToken(SyntaxKind::SlashToken, _position++, "/");
            } else if (getCurrent() == '('){
                return SyntaxToken(SyntaxKind::OpenParantesisToken, _position++, "(");
            } else if (getCurrent() == ')'){
                return SyntaxToken(SyntaxKind::CloseParantesisToken, _position++, ")");
            }

            return SyntaxToken(SyntaxKind::BadToken, _position++, _text.substr(_position - 1, 1));
        }

    private:
        std::string _text;
        //Позиция указателя на текущий символ текста
        int _position = 0;

        //Текущий символ текста
        char _current;
        //Функция, получающая текущий символ текста, исходя из значений _position.
        //Если указатель подошел к концу текста или перешел конец, то возвращается символ '\0'
        char getCurrent(){
            if (_position >= _text.length()){
                return '\0';
            }
            return _text[_position];
        }

        //Переходит к следующей позиции в тексте
        void Next(){
            _position++;
        }
};

/*
Класс, содержащий выражение, в котором определен
один токен с числом.
В качестве дочернего элемента в дереве содержит токен типа NumberToken.
*/
class NumberExpressionSyntax : public ExpressionSyntax{
    public:
        NumberExpressionSyntax(SyntaxToken numberToken){
            NumberToken = numberToken;
        }

        SyntaxKind GetKind() override {
            return SyntaxKind::NumberExpression;
        }

        SyntaxToken NumberToken;

        std::vector<std::unique_ptr<SyntaxNode>> GetChildren() override {
            std::vector<std::unique_ptr<SyntaxNode>> numberVector;
            numberVector.push_back(std::make_unique<SyntaxToken>(NumberToken));
            return numberVector;
        }
};

/*
Класс, представляющий бинарное выражение.
Содержит в себе 2 выражения и оператор между ними.
В дереве строится так
     operator
       /  \
    left  right
Содержит 2 подвыражения и оператор в качестве дочерних элементов
*/
class BinaryExpressionSyntax : public ExpressionSyntax{
    public:
        BinaryExpressionSyntax(ExpressionSyntax left, SyntaxToken operatorToken, ExpressionSyntax right){
            Left = left;
            OperatorToken = operatorToken;
            Right = right;
        }

        ExpressionSyntax Left;
        SyntaxToken OperatorToken;
        ExpressionSyntax Right;

        SyntaxKind GetKind() override {
            return SyntaxKind::BinaryExpression;
        }

        std::vector<std::unique_ptr<SyntaxNode>> GetChildren() override {
            std::vector<std::unique_ptr<SyntaxNode>> binaryExpressionVector;
            binaryExpressionVector.push_back(std::make_unique<ExpressionSyntax>(Left));
            binaryExpressionVector.push_back(std::make_unique<SyntaxToken>(OperatorToken));
            binaryExpressionVector.push_back(std::make_unique<ExpressionSyntax>(Right));
            return binaryExpressionVector;
        }
};

/*
Класс, представляющий синтаксический анализатор/парсер, который
строит синтаксическое дерево исходя из получаемых из
лексического анализатора токенов
*/
class Parser{
    //Строит синтаксическое дерево исходя из токенов
    public:
        /*
        В данном конструкторе происходит сбор всех токенов
        в отдельную коллекцию _tokens, из которой будут 
        браться токены для анализа
        */
        Parser(std::string text){
            std::vector<SyntaxToken> tokens;

            Lexer lexer = Lexer(text);
            SyntaxToken token;
            do{
                token = lexer.NextToken();

                if (token.Kind != SyntaxKind::WhitespaceToken &&
                    token.Kind != SyntaxKind::BadToken){
                        tokens.push_back(token);
                    }

            } while (token.Kind != SyntaxKind::EndOfFileToken);
            _tokens = tokens;
        }

        //Метод, реализующий парсер (пока еще не понял, по какой логике работает)
        ExpressionSyntax Parse(){
            ExpressionSyntax left = ParsePrimeryExpression();

            while (getCurrent().Kind == SyntaxKind::PlusToken ||
                getCurrent().Kind == SyntaxKind::MinusToken)
            {
                SyntaxToken operatorToken = NextToken();
                ExpressionSyntax right = ParsePrimeryExpression();
                left = BinaryExpressionSyntax(left, operatorToken, right);
            }
            
            return left;
        }

        //Пока не понял логику
        ExpressionSyntax ParsePrimeryExpression(){
            SyntaxToken numberToken = Match(SyntaxKind::NumberToken);
            return NumberExpressionSyntax(numberToken);
        }

    private:
        std::vector<SyntaxToken> _tokens;
        int _position;
        
        //Получает текущий токен
        SyntaxToken getCurrent(){
            return Peek(0);
        }

        //Получает текущий токен и переходит к следующему
        SyntaxToken NextToken(){
            SyntaxToken current = getCurrent();
            _position++;
            return current;
        }

        //Если тип текущего токена совпадает с тем, что указан в параметре, то
        //возвращает текущий токен и переходит к следующему. Если же нет, то
        //возвращает новый токен типа из параметра, с позицией текущего токена и пустой лексемой
        SyntaxToken Match(SyntaxKind kind){
            if (getCurrent().Kind == kind)
                return NextToken();
            return SyntaxToken(kind, getCurrent().Position, "");
        }

        //Заглядывает на offset токенов вперед.
        //Если случилось так, что заглянул в конец текста или за него,
        //то возвращает последний токен тексте. В ином случае возвращает нужный токен
        SyntaxToken Peek(int offset){
            int index = _position + offset;
            if (index >= _tokens.size())
                return _tokens.back();

            return _tokens[index];
        }
};

//Печатает синтаксическое дерево. Пока не работает
//(проблема с наследованием и классом SyntaxNode)
void PrettyPrint(SyntaxNode& node, std::string indent = "") {
    std::cout << indent;
    std::cout << node.GetKind();
    if (auto t = dynamic_cast<const SyntaxToken*>(&node)) {
        std::cout << " " << t->Value;
    }
    indent += "    ";
    for (const auto& child : node.GetChildren()) {
        PrettyPrint(*child, indent);
    }
}

int main(int, char**){
    while (true)
    {
        std::cout << "> ";
        std::string line;
        std::getline(std::cin, line);

        if (line == "")
            return 0;

        Parser parser = Parser(line);
        ExpressionSyntax expression = parser.Parse();

        PrettyPrint(expression);

    }
    
}
