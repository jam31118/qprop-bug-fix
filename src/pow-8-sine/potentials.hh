#include <grid.h>
#include <powers.hh>
#include <cmath>
#include <iostream>

// definitions of potentials


double always_zero2(double t, int me) {
  return 0;
};

double always_zero5(double x, double y, double z, double t, int me) {
  return 0;
};

// The vector potential with sine^8 envelope
class vecpot {
  double n_c;
  double phi_cep;
  double omega;
  double E_0;
  double duration;
  double ww;
public:
  // om: angular frequency, n_cyc: number of cycles, E_max: amplitude of electric field, cep: carrier envelope phase
  vecpot(double om, double n_cyc, double E_max, double cep) : omega(om), n_c(n_cyc), E_0(E_max), phi_cep(cep) {
    duration=n_c*2*M_PI/omega;
    // angular frequency of the envelope
    ww=omega/(2.0*n_c);
  };
  double operator()(double time, int me) const {
    if ((time>0.0) && (time<duration)) {
      return E_0/omega*pow(sin(ww*time), 8)*sin(omega*time+phi_cep); // here's the vector potential
    }
    else {
      return 0;
    };
  };
  // return the duration of the laser pulse
  double get_duration() {
    return duration;
  };
  double get_Up() {
    return (E_0 * E_0)/4.0/(omega*omega);
  };
  // return time integral of the vector potential \alpha(Time)=\int_0^{Time} dt A(t)
  double integral(double Time) {
    const double w=omega;
    const double ampl = E_0/omega;
    double T=Time;
    if (Time>duration) {
      T=duration;
    };
    return -ampl*((pow(n_c, 8)-4*pow(n_c, 7)-14*pow(n_c, 6)+56*pow(n_c, 5)+49*pow(n_c, 4)-196*pow(n_c, 3)-36*pow(n_c, 2)+144*n_c)*cos(((n_c+4)*w*T+phi_cep*n_c)/n_c)
		  +((-8*pow(n_c, 8))+24*pow(n_c, 7)+168*pow(n_c, 6)-504*pow(n_c, 5)-672*pow(n_c, 4)+2016*pow(n_c, 3)+512*pow(n_c, 2)-1536*n_c)*cos(((n_c+3)*w*T+phi_cep*n_c)/n_c)
		  +(28*pow(n_c, 8)-56*pow(n_c, 7)-728*pow(n_c, 6)+1456*pow(n_c, 5)+4732*pow(n_c, 4)-9464*pow(n_c, 3)-4032*pow(n_c, 2)+8064*n_c)*cos(((n_c+2)*w*T+phi_cep*n_c)/n_c)
		  +((-56*pow(n_c, 8))+56*pow(n_c, 7)+1624*pow(n_c, 6)-1624*pow(n_c, 5)-13664*pow(n_c, 4)+13664*pow(n_c, 3)+32256*pow(n_c, 2)-32256*n_c)*cos(((n_c+1)*w*T+phi_cep*n_c)/n_c)
		  +((-56*pow(n_c, 8))-56*pow(n_c, 7)+1624*pow(n_c, 6)+1624*pow(n_c, 5)-13664*pow(n_c, 4)-13664*pow(n_c, 3)+32256*pow(n_c, 2)+32256*n_c)*cos(((n_c-1)*w*T+phi_cep*n_c)/n_c)
		  +(28*pow(n_c, 8)+56*pow(n_c, 7)-728*pow(n_c, 6)-1456*pow(n_c, 5)+4732*pow(n_c, 4)+9464*pow(n_c, 3)-4032*pow(n_c, 2)-8064*n_c)*cos(((n_c-2)*w*T+phi_cep*n_c)/n_c)
		  +((-8*pow(n_c, 8))-24*pow(n_c, 7)+168*pow(n_c, 6)+504*pow(n_c, 5)-672*pow(n_c, 4)-2016*pow(n_c, 3)+512*pow(n_c, 2)+1536*n_c)*cos(((n_c-3)*w*T+phi_cep*n_c)/n_c)
		  +(pow(n_c, 8)+4*pow(n_c, 7)-14*pow(n_c, 6)-56*pow(n_c, 5)+49*pow(n_c, 4)+196*pow(n_c, 3)-36*pow(n_c, 2)-144*n_c)*cos(((n_c-4)*w*T+phi_cep*n_c)/n_c)
		  +(70*pow(n_c, 8)-2100*pow(n_c, 6)+19110*pow(n_c, 4)-57400*pow(n_c, 2)+40320)*cos(w*T+phi_cep)-40320*cos(phi_cep))/double((256*pow(n_c, 8)-7680*pow(n_c, 6)+69888*pow(n_c, 4)-209920*pow(n_c, 2)+147456)*w);  // here's the excursion alpha
  };

  // print alpha(t) calculated from the analytical expression and by trapezoidal rule (dt: time step for numerical quadrature, freq: write output every freq time steps)
  void debugIntegral(double dt, long freq) {
    std::ofstream debug_alpha("debug-alpha.dat");
    long steps(2+duration/dt);
    double alpha_trapez(0.0);
    for (long i(1); i<steps; i++) {
      const double time=double(i)*dt;
      alpha_trapez+=operator()(time, 0)*0.5;
      if (i%freq==0) {
	debug_alpha << time << " " << alpha_trapez*dt << " " << integral(time) << " " << alpha_trapez*dt-integral(time) << std::endl;
      };
      alpha_trapez+=operator()(time, 0)*0.5;
    };
  };
};                                                                                   

// the binding potential in the Hamiltonian
class scalarpot {
  double nuclear_charge;
  double R_co;
public:
  // charge: nuclear charge, co: cut-off radius
  scalarpot(double charge, double co) : nuclear_charge(charge), R_co(co) {
    //
  };
  double operator()(double x, double y, double z, double time, int me) const {
    // Coulomb potential with a cut-off; first -1/r (r<R_co) then linear (R_co<=r<2 R_co) then zero (r>R_co)
    const double m=1.0/pow2(R_co);
    double result=(x<R_co)?-1.0/x:((x<2*R_co)?-1.0/R_co+m*(x-R_co):0.0);
    return result;
  };
  double get_nuclear_charge() {return nuclear_charge; };
};

// the imaginary potential that prevents reflection at the grid boundary
class imagpot {
  long imag_potential_width;
  double ampl_im;  // amplitude of imaginary absorbing potential 100.0 imag pot on,  0.0 off
public:
  // width: width of the region where the imaginary potential is non zero, ampl: amplitude of the imaginary potential
  imagpot(long width, double ampl=100.0) : ampl_im(ampl), imag_potential_width(width) {
    //
  };
  // imaginary potential depends only on radial coordinate (yindex, zindex, time are ignored)
  double operator()(long xindex, long yindex, long zindex, double time, grid g) {
    if (ampl_im>1.0) {
      const long imag_start=g.ngps_x()-imag_potential_width;
      if (xindex<imag_start)
	return 0;
      else {
	const double r=double(xindex-imag_start)/double(imag_potential_width);
	return ampl_im*pow2(pow2(pow2(pow2(r))));
      };
    }
    else {
      return 0.0;
    };
  };
};
