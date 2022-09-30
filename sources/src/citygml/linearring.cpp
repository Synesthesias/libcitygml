#include <citygml/linearring.h>
#include <citygml/texturecoordinates.h>
#include <citygml/texture.h>
#include <citygml/texturetargetdefinition.h>
#include <citygml/citygmllogger.h>
#include <float.h>
#include <assert.h>

namespace citygml {

    LinearRing::LinearRing(const std::string& id, bool isExterior) : Object( id ), m_exterior( isExterior )
    {

    }

    bool LinearRing::isExterior() const
    {
        return m_exterior;
    }

    unsigned int LinearRing::size() const
    {
        return m_vertices.size();
    }

    void LinearRing::addVertex(const TVec3d& v)
    {
        m_vertices.push_back( v );
    }

    TVec3d LinearRing::computeNormal() const
    {
        unsigned int len = size();
        if ( len < 3 ) return TVec3d();

        // Convert latitude, longitude to diff of X, diff of Y from the first point, which X, Y is in rectangular coordinate system.
        // Assuming Earth is a ball which radian is 6378137m.
        auto vec_xyz = std::vector<TVec3d>();
        vec_xyz.reserve(len);
        constexpr double r = 6378137;
        constexpr double pi2 = 3.141592653589 * 2.0;
        auto ref_lat = m_vertices[0].x / pi2;
        auto ref_lon = m_vertices[0].y / pi2;
        auto sin_ref_lat = std::sin(ref_lat);
        auto cos_ref_lat = std::cos(ref_lat);
        auto cos_ref_lon = std::cos(ref_lon);
        for (unsigned int i=0; i < len; i++){
            auto sin_lat = std::sin(m_vertices[i].x / pi2);
            auto cos_lat = std::cos(m_vertices[i].x / pi2);
            auto cos_lon = std::cos(m_vertices[i].y / pi2);
            auto diff_y = r * (sin_lat - sin_ref_lat);
            auto diff_x = r * (cos_lat * cos_lon - cos_ref_lat * cos_ref_lon);
            vec_xyz.emplace_back(diff_y, diff_x, m_vertices[i].z);
        }

        // Calculate normal.
        // Tampieri, F. 1992. Newell's method for computing the plane equation of a polygon. In Graphics Gems III, pp.231-232.
        TVec3d n( 0., 0., 0. );
        for ( unsigned int i = 0; i < len; i++ )
        {
            const TVec3d& current = vec_xyz[i];
            const TVec3d& next = vec_xyz[ ( i + 1 ) % len];

            n.x += ( current.y - next.y ) * ( current.z + next.z );
            n.y += ( current.z - next.z ) * ( current.x + next.x );
            n.z += ( current.x - next.x ) * ( current.y + next.y );
        }
        return n.normal();
    }

    std::vector<TVec3d>& LinearRing::getVertices()
    {
        return m_vertices;
    }

    void LinearRing::setVertices(std::vector<TVec3d> vertices)
    {
        m_vertices = vertices;
    }

    const std::vector<TVec3d>& LinearRing::getVertices() const
    {
        return m_vertices;
    }

    void LinearRing::removeDuplicateVertices(const std::vector<TextureTargetDefinition*>& targets, std::shared_ptr<CityGMLLogger> logger )
    {
        // Currently TextureCoordinates sharing via xlink is not supported (every TextureTargetDefinition is the
        // sole owner of its TextureCoordinate objects... if this ever changes use an unordered_set for the texture coordinates
        std::vector<std::shared_ptr<TextureCoordinates> > coordinatesList;

        bool textureCoordinatesVerticesMismatch = false;

        for (auto& texTarget : targets) {

            for (unsigned int i = 0; i < texTarget->getTextureCoordinatesCount(); i++) {
                auto texCoords = texTarget->getTextureCoordinates(i);

                if (texCoords->targets(*this)) {

                    coordinatesList.push_back(texCoords);

                    if (m_vertices.size() != texCoords->getCoords().size()) {
                        CITYGML_LOG_WARN(logger, "Number of vertices in LinearRing with id '" << this->getId() << "' (" <<
                                         m_vertices.size() << ") does not match with number of texture coordinates in coordinates list "
                                         << " with id '" << texCoords->getId() << "' (" << texCoords->getCoords().size() << ")");
                        textureCoordinatesVerticesMismatch = true;
                    }
                }
            }

        }

        // Remove duplicated vertex
        unsigned int len = m_vertices.size();
        if ( len < 2 ) return;

        unsigned int i = 0;
        while ( i < m_vertices.size() && m_vertices.size() > 2 )
        {
            if ( ( m_vertices[i] - m_vertices[ ( i + 1 ) % m_vertices.size() ] ).sqrLength() <= DBL_EPSILON )
            {
                m_vertices.erase( m_vertices.begin() + i );
                for (auto coordinates : coordinatesList) {
                    coordinates->eraseCoordinate(i);
                }
            } else {
                i++;
            }
        }

#ifndef NDEBUG
        // Check integrity after duplicate removel... no need when the input was already corrupted
        if (textureCoordinatesVerticesMismatch) {
            return;
        }

        for (auto coordinates : coordinatesList) {
            if (coordinates->getCoords().size() != m_vertices.size()) {
                CITYGML_LOG_ERROR(logger, "Broken implementation. Duplicate vertex removal in LinearRing with id '" << this->getId()
                                  << "' caused a mismatch of texture coordinates in coordinates list  with id '" << coordinates->getId()
                                  << "' (" << m_vertices.size() << " != " <<coordinates->getCoords().size() << ")");
            }
        }
#endif
    }

    void LinearRing::forgetVertices()
    {
        m_vertices.clear();
    }

}
