# Schwarzschild Geodesic Refactor – Physics Requirements

## Objectives
1. Provide an exact implementation of the Schwarzschild metric in geometric units (G = c = 1) so that **all tensors match the analytic solution**.
2. Supply the renderer with null geodesic solutions that respect:
   - the null constraint \( g_{\mu\nu} p^\mu p^\nu = 0 \),
   - conservation of energy and angular momentum,
   - full Runge–Kutta integration of the 4-position and 4-momentum.
3. Establish ground truth checks (analytic lensing limits, photon sphere impact parameter, etc.) that prevent regressions.
4. Extend the infrastructure so the Kerr metric (rotating black holes) plugs into the same tensor/Christoffel machinery without bespoke shader code.

## Coordinates and Conventions
- Use Schwarzschild coordinates \(x^\mu = (t, r, \theta, \phi)\).
- Adopt signature \((- + + +)\).
- Motor the metric with the dimensionless mass parameter \(M\) (solar masses expressed in geometric units).
- The metric’s event horizon radius is \(r_s = 2M\).

## Metric Tensor

\[
ds^2 = -\left(1 - \frac{2M}{r}\right) dt^2 + \left(1 - \frac{2M}{r}\right)^{-1} dr^2 + r^2 d\theta^2 + r^2 \sin^2 \theta\, d\phi^2
\]

Non-zero components:

\[
g_{tt} = -\left(1 - \frac{2M}{r}\right), \quad
g_{rr} = \left(1 - \frac{2M}{r}\right)^{-1}, \quad
g_{\theta\theta} = r^2, \quad
g_{\phi\phi} = r^2 \sin^2 \theta
\]

Inverse:

\[
g^{tt} = -\left(1 - \frac{2M}{r}\right)^{-1}, \quad
g^{rr} = \left(1 - \frac{2M}{r}\right), \quad
g^{\theta\theta} = \frac{1}{r^2}, \quad
g^{\phi\phi} = \frac{1}{r^2 \sin^2 \theta}
\]

## Christoffel Symbols

All non-zero components (up to symmetry) required for null geodesics:

\[
\begin{aligned}
&\Gamma^{t}_{tr} = \Gamma^{t}_{rt} = \frac{M}{r(r - 2M)} \\
&\Gamma^{r}_{tt} = \frac{M}{r^3} (r - 2M) \\
&\Gamma^{r}_{rr} = -\frac{M}{r(r - 2M)} \\
&\Gamma^{r}_{\theta\theta} = -(r - 2M) \\
&\Gamma^{r}_{\phi\phi} = -(r - 2M) \sin^2 \theta \\
&\Gamma^{\theta}_{r\theta} = \Gamma^{\theta}_{\theta r} = \frac{1}{r} \\
&\Gamma^{\theta}_{\phi\phi} = -\sin \theta \cos \theta \\
&\Gamma^{\phi}_{r\phi} = \Gamma^{\phi}_{\phi r} = \frac{1}{r} \\
&\Gamma^{\phi}_{\theta\phi} = \Gamma^{\phi}_{\phi\theta} = \cot \theta
\end{aligned}
\]

The integrator must compute these exactly (no approximations, no smoothing near the horizon) and handle the coordinate singularity explicitly via adaptive stepping and analytic regularisation if needed.

## Conserved Quantities

For null geodesics:
- Specific energy: \(E = -g_{tt} \, p^t\).
- Angular momentum about the z-axis: \(L_z = g_{\phi\phi} \, p^\phi\).
- Carter constant is zero in Schwarzschild, reducing to total angular momentum squared.

Initial conditions generated from the camera must satisfy:
\[
g_{\mu\nu} \, p^\mu p^\nu = 0
\]

Use these invariants to monitor numerical drift and reject steps that violate them beyond tolerance.

## Geodesic Evolution

Advance the state vector \(Y = (x^\mu, p^\mu)\) according to:
\[
\frac{dx^\mu}{d\lambda} = p^\mu, \qquad
\frac{dp^\mu}{d\lambda} = -\Gamma^{\mu}_{\alpha\beta} p^\alpha p^\beta
\]
with \(p^\mu\) enforcing the null constraint after each RK4 sub-step (renormalise using \(p^t\) derived from \(E\)).

## Validation Targets

1. **Photon sphere orbit**: initialise \(r = 3M\), \(\theta = \pi/2\), \(p^r = 0\), choose \(L/E = \sqrt{27} M\). Expect constant radius trajectory within tolerance.
2. **Deflection angle**: for impact parameter \(b = 10M\), compare scattering with analytic expansion \( \Delta \phi \approx 4M/b + \mathcal{O}((M/b)^2) \).
3. **Shadow radius**: rays from infinity with \(b < \sqrt{27}M\) should fall into the black hole; others should escape.

These tests must be automated so the renderer only ships if all checks pass.
