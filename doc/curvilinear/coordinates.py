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
    return np.dot(la.inv(self.K(coords)), lgrad)

  def grad_vector(self, coords, v, lgrad):
    """
      Take a vector in standard coordinates, apply the gradient, 
      and return to standard coordinates

      Parameters:
        coords      current coordinate values
        v           vector in standard coordinates
        lgrad       local 
    """
    K = self.K(coords)
    Ki = la.inv(K)
    g = self.g(coords)
    
    print("HMM")
    for i in range(3):
      print(i)
      print(v)
      print(self.L(coords,i))
      print(np.dot(v,self.L(coords,i)))

    return np.vstack(list(np.dot(np.dot(la.inv(g),v), self.L(coords, i)) for i in range(3)))


class Cartesian(CoordinateSystem):
  """
    The coordinates are (x1, x2, x3...)
  """
  def __init__(self, *args, **kwargs):
    super().__init__(*args, **kwargs)

  def h(self, coords):
    return np.ones((self.dim,))

class Cylindrical(CoordinateSystem):
  """
    The coordinates are (r, theta, z)
  """
  def __init__(self, *args, **kwargs):
    super().__init__(3, *args, **kwargs)

  def h(self, coords):
    return np.array([1,coords[0],1])
