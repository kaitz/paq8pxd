#pragma once
///////////////// Ordinary Least Squares predictor /////////////////

template <typename F, typename T, const bool hasZeroMean = true>
class OLS {
  static constexpr F ftol = 1E-8;
  static constexpr F sub = F(int64_t(!hasZeroMean)<<(8*sizeof(T)-1));
private:
  int n, kmax, km, index;
  F lambda, nu;
  F *x, *w, *b;
  F **mCovariance, **mCholesky;
  int Factor() {
    // copy the matrix
    for (int i=0; i<n; i++)
      for (int j=0; j<n; j++)
        mCholesky[i][j] = mCovariance[i][j];

    for (int i=0; i<n; i++)
      mCholesky[i][i] += nu;
    for (int i=0; i<n; i++) {
      for (int j=0; j<i; j++) {
        F sum = mCholesky[i][j];
        for (int k=0; k<j; k++)
          sum -= (mCholesky[i][k] * mCholesky[j][k]);
        mCholesky[i][j] = sum / mCholesky[j][j];
      }
      F sum = mCholesky[i][i];
      for (int k=0; k<i; k++)
        sum -= (mCholesky[i][k] * mCholesky[i][k]);
      if (sum>ftol)
        mCholesky[i][i] = sqrt(sum);
      else
        return 1;
    }
    return 0;
  }

  void Solve() {
    for (int i=0; i<n; i++) {
      F sum = b[i];
      for (int j=0; j<i; j++)
        sum -= (mCholesky[i][j] * w[j]);
      w[i] = sum / mCholesky[i][i];
    }
    for (int i=n-1; i>=0; i--) {
      F sum = w[i];
      for (int j=i+1; j<n; j++)
        sum -= (mCholesky[j][i] * w[j]);
      w[i] = sum / mCholesky[i][i];
    }
  }
public:
  OLS(int n, int kmax=1, F lambda=0.998, F nu=0.001) : n(n), kmax(kmax), lambda(lambda), nu(nu) {
    km = index = 0;
    x = new F[n], w = new F[n], b = new F[n];
    mCovariance = new F*[n], mCholesky = new F*[n];
    for (int i=0; i<n; i++) {
      x[i] = w[i] = b[i] = 0.;
      mCovariance[i] = new F[n], mCholesky[i] = new F[n];
      for (int j=0; j<n; j++)
        mCovariance[i][j] = mCholesky[i][j] = 0.;
    }
  }
  ~OLS() {
    delete x, delete w, delete b;
    for (int i=0; i<n; i++) {
      delete mCovariance[i];
      delete mCholesky[i];
    }
    delete[] mCovariance, delete[] mCholesky;
  }
  void Add(const T val) {
    if (index<n)
      x[index++] = F(val)-sub;
  }
  void AddFloat(const F val) {
    if (index<n)
      x[index++] = val-sub;
  }
  F Predict(const T **p) {
    F sum = 0.;
    for (int i=0; i<n; i++)
      sum += w[i] * (x[i] = F(*p[i])-sub);
    return sum+sub;
  }
  F Predict() {
    assert(index==n);
    index = 0;
    F sum = 0.;
    for (int i=0; i<n; i++)
      sum += w[i] * x[i];
    return sum+sub;
  }
  void Update(const T val) {
    for (int j=0; j<n; j++)
      for (int i=0; i<n; i++)
        mCovariance[j][i] = lambda * mCovariance[j][i] + (1.0 - lambda) * (x[j] * x[i]);
    for (int i=0; i<n; i++)
      b[i] = lambda * b[i] + (1.0 - lambda) * (x[i] * (F(val)-sub));
    km++;
    if (km>=kmax) {
      if (!Factor()) Solve();
      km = 0;
    }
  }
};
