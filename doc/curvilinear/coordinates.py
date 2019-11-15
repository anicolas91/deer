import numpy as np
import numpy.linalg as la

class CoordinateSystem:
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

  def grad_vec(self, coords, v, lgrad):
    """
      Take a vector in standard coordinates, apply the gradient, 
      and return to standard coordinates

      Parameters:
        coords      current coordinate values
        v           vector in standard coordinates
        lgrad       local 
    """
    return np.dot(np.dot(la.inv(self.K(coords)), (np.dot(lgrad, la.inv(self.K(coords))) + sum(
        np.dot(self.L(coords, i), np.dot(K(coords), v)) 
        for i in range(self.dim)))), la.inv(self.K(coords).T))

class Cartesian(CoordinateSystem):
  """
    The coordinates are (x1, x2, x3...)
  """
  def __init__(self, *args, **kwargs):
    super().__init__(*args, **kwargs)

  def g(self, coords):
    """
      The covariant metric components
    """
    return np.eye(self.dim)

  def K(self, coords):
    """
      Transformation from standard to covariant
    """
    return np.eye(3)

  def L(self, coords, i):
    """
      The Christoffel symbols of the second kind
    """
    return np.zeros((3,3))

class Cylindrical(CoordinateSystem):
  """
    The coordinates are (r, theta, z)
  """
  def __init__(self, *args, **kwargs):
    super().__init__(3, *args, **kwargs)

  def g(self, coords):
    """
      The covariant metric components
    """
    return np.array([
      [1.0, 0, 0], 
      [0, coords[0]**2.0, 0],
      [0, 0 , 1.0]])

  def K(self, coords):
    """
      Transformation from standard to covariant
    """
    return np.array([
      [1.0,0.0,0.0],
      [0.0,coords[0],0],
      [0.0,0.0,1.0]])

  def L(self, coords, i):
    """
      The Christoffel symbols of the second kind
    """
    if i == 0:
      return np.array([
        [0,0,0],
        [0,-coords[0],0],
        [0,0,0.0]])
    elif i == 1:
      return np.array([
        [0,1.0/coords[0],0],
        [1.0/coords[0],0,0],
        [0,0,0]])
    elif i == 2:
      return np.zeros((3,3))
    else:
      raise ValueError("Cylindrical coordinates have dim = 3!")
