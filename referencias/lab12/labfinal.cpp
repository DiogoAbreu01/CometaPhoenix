#include <iostream>    // std::cout, std::boolalpha
#include <stdexcept>   // std::invalid_argument

// -----------------------------------------------------------------------------
// 1. ANTES DO TEMPLATE: duas funcoes, mesma regra, tipos diferentes.
// Estas duas versoes assumem que low <= high (nao verificam a faixa).
// -----------------------------------------------------------------------------
int clamp_int(int value, int low, int high) {
    if (value < low) return low;     // Pedido abaixo: usa o limite inferior.
    if (high < value) return high;   // Pedido acima: usa o limite superior.
    return value;                    // Dentro: conserva o pedido.
}

double clamp_double(double value, double low, double high) {
    if (value < low) return low;
    if (high < value) return high;
    return value;
}

// -----------------------------------------------------------------------------
// 2. UMA REGRA PARA VARIOS TIPOS.
// T e o tipo que sera escolhido para uma chamada concreta.
// const T&: referencia constante; nao modifica argumentos do tipo T.
// Retorno T: devolve um valor independente (nao altera o chamador).
// Pre-condicao: numeros finitos ou tipos com uma ordem coerente.
// -----------------------------------------------------------------------------
template <typename T>
T clamp_value(const T& value, const T& low, const T& high) {
    if (high < low) {
        // Isso e um problema nos DADOS, detectado durante a execucao.
        throw std::invalid_argument{"clamp range is inverted"};
    }
    if (value < low) return low;
    if (high < value) return high;
    return value;
}

// -----------------------------------------------------------------------------
// 3. MACRO X TEMPLATE: chegam ao mesmo resultado por mecanismos diferentes.
// Uma macro substitui texto; o template e uma funcao verificada para T.
// Cuidado: a macro pode avaliar argumentos mais de uma vez.
// -----------------------------------------------------------------------------
#define MENOR(a, b) (((a) < (b)) ? (a) : (b))

template <typename T>
T menor(T a, T b) {
    return a < b ? a : b;
}

// -----------------------------------------------------------------------------
// 4. UM TIPO CRIADO POR NOS: Duration.
// Para usar clamp_value, o tipo precisa permitir comparacoes com < e retorno T.
// Nao precisa herdar de nenhuma classe, somar ou usar cout.
// -----------------------------------------------------------------------------
namespace ecop13a {

class Duration {
public:
    explicit Duration(long long ms) : ms_{ms} {}

    long long milliseconds() const noexcept { return ms_; }

private:
    long long ms_;
};

// O significado de "menor" e definido pelo dominio: menos milissegundos.
// Aqui, os operandos sao recebidos por valor, como no slide 21.
bool operator<(Duration left, Duration right) noexcept {
    return left.milliseconds() < right.milliseconds();
}

}  // namespace ecop13a

// -----------------------------------------------------------------------------
// 5. UM TIPO SEM ORDEM DEFINIDA: Point.
// O compilador nao pode decidir sozinho como comparar pontos.
// As funcoes abaixo mostram DOIS criterios distintos; nao criamos operator<.
// -----------------------------------------------------------------------------
struct Point {
    int x;
    int y;
};

bool menor_por_x(const Point& a, const Point& b) {
    return a.x < b.x;
}

long long distancia_quadrada(const Point& p) {
    // O quadrado dispensa raiz quadrada e basta para comparar distancias.
    return 1LL * p.x * p.x + 1LL * p.y * p.y;
}

bool menor_por_distancia(const Point& a, const Point& b) {
    return distancia_quadrada(a) < distancia_quadrada(b);
}

// -----------------------------------------------------------------------------
// 6. EXERCICIO DA AULA: pertence ao intervalo FECHADO [low, high]?
// Usa apenas <, como solicitado. Nas fronteiras, retorna true.
// O && tem curto-circuito: se a esquerda for false, a direita nao e avaliada.
// -----------------------------------------------------------------------------
template <typename T>
bool is_between(const T& value, const T& low, const T& high) {
    if (high < low) {
        throw std::invalid_argument{"is_between range is inverted"};
    }
    return !(value < low) && !(high < value);
}

// Usada SOMENTE para tornar o curto-circuito visivel na saida.
bool verificar_limite_superior(int value, int high) {
    std::cout << "    [O segundo teste foi executado]\n";
    return !(high < value);
}

void titulo(const char* texto) {
    std::cout << "\n=== " << texto << " ===\n";
}

int main() {
    // Mostra booleanos como true/false, em vez de 1/0.
    std::cout << std::boolalpha;

    titulo("1. Duas funcoes repetidas (slides 4-8)");
    std::cout << "clamp_int(17, 0, 10) = " << clamp_int(17, 0, 10) << '\n';
    std::cout << "clamp_double(2.4, 0.0, 5.0) = "
              << clamp_double(2.4, 0.0, 5.0) << '\n';
    // PERGUNTA: O que mudou entre as duas funcoes? Somente o tipo.

    titulo("2. Uma funcao template (slides 9-12)");
    int solicitado = 17;
    int aplicado = clamp_value(solicitado, 0, 10);  // T deduzido como int.
    std::cout << "solicitado = " << solicitado << '\n'; // Continua 17.
    std::cout << "aplicado = " << aplicado << '\n';     // Recebe 10.
    std::cout << "Abaixo de 0: " << clamp_value(-3, 0, 10) << '\n';
    std::cout << "Dentro da faixa: " << clamp_value(2.4, 0.0, 5.0) << '\n';
    // PERGUNTA: Por que 'solicitado' nao foi modificado?

    titulo("3. Deducao do tipo T (slides 13-16)");
    // DESCOMENTE PARA VER UM ERRO DE COMPILACAO (deducoes em conflito):
    // auto erro_deducao = clamp_value(1, 0.0, 2.0);

    std::cout << "T = double explicito (retorno do tipo double): "
              << clamp_value<double>(2, 0, 5) << '\n';
    // Primeiro T e escolhido; depois 2, 0 e 5 podem virar double.

    double inferior = 0.0;
    double superior = 10.5;
    // DESCOMENTE PARA VER O MESMO CONFLITO DO EXERCICIO 1:
    // auto erro_exercicio = clamp_value(solicitado, inferior, superior);

    std::cout << "Conserva faixa [0,10.5]: "
              << clamp_value<double>(solicitado, inferior, superior) << '\n';
    std::cout << "Muda para faixa [0,10]: "
              << clamp_value(solicitado, 0, 10) << '\n';
    // PERGUNTA: Ambas compilam. Qual preserva o requisito original?

    titulo("4. Macro versus template (slide 18)");
    std::cout << "MENOR(3, 5) = " << MENOR(3, 5) << '\n';
    std::cout << "menor(3, 5) = " << menor(3, 5) << '\n';
    // A macro expande tokens; menor<int> e uma funcao concreta.

    titulo("5. Tipo Duration: capacidades exigidas (slides 19-25)");
    using ecop13a::Duration;
    Duration duracao_pedida{2500};
    Duration duracao_minima{500};
    Duration duracao_maxima{2000};
    auto duracao_aplicada =
        clamp_value(duracao_pedida, duracao_minima, duracao_maxima);
    std::cout << "Pedido: " << duracao_pedida.milliseconds() << " ms\n";
    std::cout << "Resultado: " << duracao_aplicada.milliseconds() << " ms\n";
    // O corpo so exige < (e um retorno T que possa ser construido).

    titulo("6. Point: a ordem depende do significado (slides 26-30)");
    Point p{1, 100};
    Point q{2, 0};
    std::cout << "P < Q por x? " << menor_por_x(p, q) << '\n';
    std::cout << "P < Q por distancia? " << menor_por_distancia(p, q)
              << '\n';
    std::cout << "Distancias ao quadrado: P=" << distancia_quadrada(p)
              << ", Q=" << distancia_quadrada(q) << '\n';

    // DESCOMENTE AS DUAS LINHAS PARA VER UM ERRO DE COMPILACAO:
    // Point low_point{0, 0}, high_point{2, 0};
    // auto erro_point = clamp_value(p, low_point, high_point);
    // T seria Point, mas o corpo exigiria operator<, que nao existe.
    // Uma ordem artificial poderia compilar e mesmo assim estar errada.

    titulo("7. Limitar, recusar e erros (slides 31-33)");
    const int pedido = 17;
    const int low = 0;
    const int high = 10;
    std::cout << "Politica limitar: " << clamp_value(pedido, low, high)
              << '\n';
    if (pedido < low || high < pedido) {
        std::cout << "Politica recusar: Pedido recusado\n";
    } else {
        std::cout << "Politica recusar: Pedido aceito\n";
    }

    // Isto COMPILA, mas a faixa invertida provoca excecao na EXECUCAO.
    try {
        std::cout << clamp_value(2, 10, 0) << '\n';
    } catch (const std::invalid_argument& erro) {
        std::cout << "Erro em execucao: " << erro.what() << '\n';
    }
    // Um erro de compilacao nao pode ser resolvido com catch.

    titulo("8. is_between e curto-circuito (slides 34-36)");
    std::cout << "-1 em [0,10]? " << is_between(-1, 0, 10) << '\n';
    std::cout << " 0 em [0,10]? " << is_between( 0, 0, 10) << '\n';
    std::cout << "10 em [0,10]? " << is_between(10, 0, 10) << '\n';
    std::cout << "11 em [0,10]? " << is_between(11, 0, 10) << '\n';
    std::cout << "2000 ms em [500,2000] ms? "
              << is_between(Duration{2000}, Duration{500}, Duration{2000})
              << '\n';

    // Demonstracao do MESMO &&, agora com mensagem no segundo teste.
    std::cout << "Curto-circuito com value=-1:\n";
    bool teste_a = !(-1 < 0) && verificar_limite_superior(-1, 10);
    std::cout << "    Resultado: " << teste_a << '\n';
    std::cout << "Curto-circuito com value=0:\n";
    bool teste_b = !(0 < 0) && verificar_limite_superior(0, 10);
    std::cout << "    Resultado: " << teste_b << '\n';

    try {
        std::cout << is_between(5, 10, 0) << '\n';
    } catch (const std::invalid_argument& erro) {
        std::cout << "Faixa invalida em is_between: " << erro.what() << '\n';
    }

    std::cout << "\nIdeia central: generalize a regra, preserve o significado.\n";
    return 0;
}