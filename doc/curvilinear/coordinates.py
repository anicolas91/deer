import numpy as np
import numpy.linalg as la
import scipy.linalg as sla

class OrthogonalCoordinateSystem:
  """
    Generic superclass for all coordinate systems
  """
  def __init__(self, dim):
    self.dim = dim

  def grad_scalar(self, coords, s, lgrad):
    """
      Take a scalar, apply the gradient, and return to 
      standard coordinates
    """
    return lgrad / self.h(coords)

  def grad_vector(self, coords, v, lgrad):
    """
      Take a vector in standard coordinates, apply the gradient, 
      and return to standard coordinates

      Parameters:
        coords      current coordinate values
        v           vector in standard coordinates
        lgrad       local 
    """
    G = np.zeros((self.dim,self.dim))

    h = self.h(coords)
    dh = self.dh(coords)

    for i in range(self.dim):
      for j in range(self.dim):
        if i == j:
          G[i,j] = lgrad[i,i] / h[i] - v[i]/h[i]**2.0 * dh[i,i] + np.dot(dh, v)[i] / h[i]
        else:
          G[i,j] = (lgrad[i,j] - v[j]/h[i] * dh[j,i]) / h[j]

    return G

class Cartesian(OrthogonalCoordinateSystem):
  """
    The coordinates are (x1, x2, x3...)
  """
  def __init__(self, *args, **kwargs):
    super().__init__(*args, **kwargs)

  def h(self, coords):
    return np.ones((self.dim,))

  def dh(self, coords):
    return np.zeros((3,3))

class Cylindrical(OrthogonalCoordinateSystem):
  """
    The coordinates are (r, theta, z)
  """
  def __init__(self, *args, **kwargs):
    super().__init__(3, *args, **kwargs)

  def h(self, coords):
    return np.array([1,coords[0],1])

  def dh(self, coords):
    return np.array([[0,0,0],[1,0,0],[0,0,0]])
