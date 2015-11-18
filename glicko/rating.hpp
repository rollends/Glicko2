#ifndef GLICKO_RATING_HPP
#define GLICKO_RATING_HPP

#define _USE_MATH_DEFINES
#include <math.h>
#include <iostream>
#include <algorithm>
#include <tuple>

#include "glicko/config.hpp"
#include "glicko/stdutil.hpp"

namespace Glicko
{
    /// Defines a struct that contains Glicko rating parameters Parameters are
    /// stored in the Glicko-2 rating scale, but can also be queried in in
    /// original Glicko scale.
    class Rating
    {
    public:
        /// Constructs a rating with from a desired rating and deviation.
        /// Defaults to the Glicko-1 standard of R = 1500, RD = 350.
        Rating(const double rating = Glicko::kDefaultR,
               const double deviation = Glicko::kDefaultRD,
               const double volatility = Glicko::kDefaultS);

        /// Constructs a rating from another rating instance
        Rating(const Rating& rating);

        /// Updates the rating based on a set of games
        template<
                    class RatingContainer,
                    class ScoreContainer,
                    typename = std::enable_if_t< type_util::is_forward_iterable< RatingContainer >::value >,
                    typename = std::enable_if_t< type_util::is_forward_iterable< ScoreContainer >::value >
                >
        void Update( RatingContainer const & opponents, ScoreContainer const & score )
        {
            unsigned long constexpr G = 0;
            unsigned long constexpr E = 1;
            using TableRow = std::tuple< double, double >;
            std::vector<TableRow> geTable;

            auto FormTable = [this]( Rating const & b ){ return std::make_tuple( b.G(), b.E(b.G(), *this) ); };
            auto AddInvV =
                [](double acc,TableRow pair){
                    double g = std::get<G>(pair);
                    double e = std::get<E>(pair);
                    return acc + g * g * e * (1.0 - e);
                };
            auto AddDInner =
                [](TableRow gepair, double score) {
                    return std::get<G>(gepair) * (score - std::get<E>(gepair));
                };

            // Initialize the g,e table
            std::transform( opponents.cbegin(), opponents.cend(), std::back_inserter( geTable ), FormTable );

            // accumulate results into invV.
            double invV = std::accumulate( geTable.cbegin(), geTable.cend(), 0.0, AddInvV );

            // Invert the v value
            double v = 1.0 / invV;

            // Compute the delta value from the g, e, and v
            // values
            double dInner =
                std::inner_product(
                    geTable.cbegin(),
                    geTable.cend(),
                    score.cbegin(),
                    0.0,
                    std::plus<double>(),
                    AddDInner
                );

            // Apply the v value to the delta
            double d = v * dInner;

            // Compute new rating, deviation and volatility values
            sPrime = exp(Convergence(d, v, p, s)/2.0);
            pPrime = 1.0 / sqrt((1.0/(p*p + sPrime*sPrime)) + invV);
            uPrime = u + pPrime * pPrime * dInner;
        }

        // Updates the rating based on a single game
        void Update(const Rating& opponent, const double score);

        // Decays the rating deviation
        void Decay();

        /// Applies the updated ratings
        void Apply();

        /// Returns the Glicko-1 rating
        inline double Rating1() const
        {
            return (u * Glicko::kScale) + Glicko::kDefaultR;
        }

        /// Returns the Glicko-1 deviation
        inline double Deviation1() const
        {
            return p * Glicko::kScale;
        }

        /// Returns the Glicko-2 rating
        inline double Rating2() const
        {
            return u;
        }

        /// Returns the Glicko-2 deviation
        inline double Deviation2() const
        {
            return p;
        }

        /// Outputs the rating in Glicko-1 fromat
        friend inline std::ostream& operator<<(std::ostream& pStream,
                                               const Rating& pRating)
        {
            pStream << "[" << pRating.Rating1()
                    << ":" << pRating.Deviation1()
                    << "]";

            return pStream;
        }

    private:
        /// Computes the value of the g function for a rating
        inline double G() const
        {
            double scale = p / M_PI;
            return 1.0 / sqrt(1.0 + 3.0 * scale * scale);
        }

        /// Computes the value of the e function in terms of a g function value
        /// and another rating
        inline double E(const double g, const Rating& rating) const
        {
            float exponent = -1.0 * g * (rating.u - u);
            return 1.0 / (1.0 + exp(exponent));
        }

        /// Computes the value of the f function in terms of x, delta^2, phi^2,
        /// v, a and tau^2.
        static inline double F(const double x,
                               const double dS,
                               const double pS,
                               const double v,
                               const double a,
                               const double tS)
        {
            double eX = exp(x);
            double num = eX * (dS - pS - v - eX);
            double den = pS + v + eX;

            return (num/ (2.0 * den * den)) - ((x - a)/tS);
        }

        /// Performs convergence iteration on the function f
        static double Convergence(const double d,
                                  const double v,
                                  const double p,
                                  const double s);

        /// The rating u (mu)
        double u;

        /// The rating deviation p (phi)
        double p;

        /// The rating volatility s (sigma)
        double s;

        /// The pending rating value, u'
        double uPrime;

        /// The pending deviation value, u'
        double pPrime;

        /// The pending volatility value, s'
        double sPrime;
    };
}

#endif
